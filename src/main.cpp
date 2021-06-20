/**
 * For the NodeMCU to be operate as a master with the ESP32 as slave
 * Contained Blocks
 *    - Websocket Server for Android (A good explanation of the WebSocket server can be found here. http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-9-websockets/)
 *    - Radio Transmission (nRF24L01) https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123/2
 *       - ctrlData is the data to control the servos
 *       - imuData is the data received from the IMUs
 */

#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <printf.h>

/**
 * Node MCU Radio Comunication
 * CE 10; CSN 9; SCK 13; MISO 12; MOSI 11;
 */ 
/*******************************************************************/
#include <RF24_config.h>
#include <nRF24L01.h>
#include <RF24.h>
#define CE_PIN D8
#define CSN_PIN D4
//const byte addresses[][6] = {"00001", "00002"};
const byte addresses[6] = "00001";
//char imuData[19] = {0};    // IMU Data received from the Slave
int imuData[20] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // to hold the two values coming from the slave
bool newData = false;
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio
/*******************************************************************/

/*unsigned long previousMillis = 0;
const long interval = 1000; 
int ledState = LOW;*/


ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// num is the current client/connection number/ID.
// ctrlData is the received data. This is a pointer not a char or char array.
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *ctrlData, size_t length)
{

    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], ctrlData);

        // send message to client
        webSocket.sendTXT(num, "Connected");
    }
    break;
    case WStype_TEXT:
        IPAddress ip = webSocket.remoteIP(num);
        bool rslt;
        rslt = radio.write( ctrlData, length );
        Serial.print("Data Sent ");
        Serial.printf("%s", ctrlData);
        if(rslt){               // Everything in this bracket is part of a two way radio
            if ( radio.isAckPayloadAvailable() ) {
                radio.read(&imuData, sizeof(imuData));
                newData = true;
            }
            else {
                Serial.println("  Acknowledge but no data ");
            }
        }
        else{Serial.println("Tx Failed");}


        //Serial.printf("%s?\n", ctrlData);
        /*for (int i = 0; i <= length; i++){
                Serial.println((char)ctrlData[i]);
            }*/

        /**
             * https://www.thethingsnetwork.org/docs/devices/bytes.html
             * https://www.reddit.com/r/arduino/comments/2pf8uw/best_method_to_extract_floats_from_strings/
             * http://www.cplusplus.com/reference/cstdlib/strtof/
             * https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
             */
        //radio.write( &ctrlData, length );
        //radio.write( (uint8_t *)ctrlData, length );
        //radio.write(ctrlData, length);
        //webSocket.sendTXT(num, ctrlData,sizeof(ctrlData),false);
        
        // Commenting out to try 2 way communication.
        /* radio.stopListening();
        radio.write(ctrlData, length);
        Serial.print("Data Sent ");
        Serial.printf("%s", ctrlData);
        Serial.println(); */
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
    radio.setDataRate( RF24_250KBPS );
    radio.enableAckPayload();
    radio.setRetries(5,5); // delay, count
    radio.openWritingPipe(addresses);
    /* radio.openWritingPipe(addresses[0]); // 00001    // This is checked
    radio.openReadingPipe(1, addresses[1]); // 00002   // This is checked
    radio.setPALevel(RF24_PA_MIN);*/
    delay(1000);
    Serial.println("Radio Communication Started!");

    //Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
}

void showData() {
    if (newData == true) {
        Serial.print("  Acknowledge data ");
        Serial.println(imuData[0]);
        Serial.println(imuData[1]);
        Serial.println(imuData[2]);
        Serial.println(imuData[3]);
        Serial.println(imuData[4]);
        Serial.println();
        newData = false;
    }
}

void loop()
{
    //const char text[] = "Hello World";
    //radio.write(&text, sizeof(text));
    //delay(1000);
    webSocket.loop();
    server.handleClient();
    /* radio.startListening();
    if (radio.available()){
        radio.read(&dataReceived, sizeof(dataReceived));
        Serial.print(dataReceived);
    } */
    //if(newData){webSocket.broadcastTXT();}     // BroadcastTXT is used to transmit data to all the WebSocket Clients.
    showData();
}