/**
 * For the NodeMCU to be operate as a master with the ESP32 as slave
 * Contained Blocks
 *    - Websocket Server for Android (A good explanation of the WebSocket server can be found here. http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-9-websockets/)
 *    - Radio Transmission (nRF24L01)
 */

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include "printf.h"

#include "nRF24L01.h"
#include "RF24.h"
#define CE_PIN D8
#define CSN_PIN D4
int ackData[2] = {-1, -1}; // to hold the two values coming from the slave
const byte addresses[][6] = {"00001", "00002"};
bool newData = false;
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// num is the current client/connection number/ID.
// payload is the received data. This is a pointer not a char or char array.
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
    }
    break;
    case WStype_TEXT:
        IPAddress ip = webSocket.remoteIP(num);
        //Serial.printf("%s?\n", payload);
        /*for (int i = 0; i <= length; i++){
                Serial.println((char)payload[i]);
            }*/

        /**
             * https://www.thethingsnetwork.org/docs/devices/bytes.html
             * https://www.reddit.com/r/arduino/comments/2pf8uw/best_method_to_extract_floats_from_strings/
             * http://www.cplusplus.com/reference/cstdlib/strtof/
             * https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
             */
        //radio.write( &payload, length );
        //radio.write( (uint8_t *)payload, length );
        //radio.write(payload, length);
        //webSocket.sendTXT(num, payload,sizeof(payload),false);
        bool rslt;
        rslt = radio.write(payload, length);
        Serial.print("Data Sent ");
        Serial.printf("%s", payload);
        Serial.println();
        /*if(rslt){               // Everything in this bracket is part of a two way radio
                if ( radio.isAckPayloadAvailable() ) {
                    radio.read(&ackData, sizeof(ackData));
                    newData = true;
                }
                else {
                    Serial.println("  Acknowledge but no data ");
                }
                //updateMessage();
            }
            else{Serial.println("Tx Failed");}*/
        break;
    }
}

void setup()
{
    Serial.begin(115200);

    pinMode(LED_BUILTIN, OUTPUT);

    // Starting Access Point (NodeMCU)
    Serial.println("Starting AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("NodeMCU Access Point", "12345678");
    Serial.println("AP Started!");

    // Start Websocket Server
    Serial.println("Starting Websocket...");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("Websocket Started!");

    // I am not sure what this is.
    if (MDNS.begin("simcontrol"))
    {
        Serial.println("MDNS responder started!");
    }
    Serial.println("Starting Server...");
    //handle index
    server.on(
        "/", []() {
            server.send(200, "text/plain", "You are connected");
        });
    server.begin();
    Serial.println("Server Started!");

    //Radio Communication (nRF24L01+LA+PNA)
    Serial.println("Radio Communnication Starting...");
    radio.begin();
    radio.openWritingPipe(addresses[1]);    // 00001
    radio.openReadingPipe(1, addresses[0]); // 00002
    radio.setPALevel(RF24_PA_MIN);
    Serial.println("Radio Communication Started!");

    //Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
}

void loop()
{
    /*unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        if (ledState == LOW) { ledState = HIGH; } 
        else { ledState = LOW; }
        digitalWrite(LED_BUILTIN, ledState);
    }*/
    webSocket.loop();
    server.handleClient();
    //if(newData){webSocket.broadcastTXT();}     // BroadcastTXT is used to transmit data to all the WebSocket Clients.
}