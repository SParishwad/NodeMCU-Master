/**
 * For the NodeMCU to be operate as a master with the ESP32 as slave
 * Contained Blocks
 *    - Websocket Server for Android (A good explanation of the WebSocket server can be found here. http://www.martyncurrey.com/esp8266-and-the-arduino-ide-part-9-websockets/)
 *    - Radio Transmission (nRF24L01) https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123/2
 *       - ctrlData is the data to control the servos
 *       - imuData is the data received from the IMUs
 */

/**
 * Standard Libraries
 */
/**
 * NodeMCU Radio Master
 * CE D8; CSN D4; SCK ; MISO ; MOSI ;
 */
#include <Arduino.h>
#include <SPI.h>
//#include <RF24_config.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN D8
#define CSN_PIN D4
const byte address[6] = "00001";
RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

int imuData[4] = {-1, -1, -1, -1};//, -1, -1, -1, -1 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}; // to hold the two values coming from the slave
bool newData = false;

/*******************************************************************/

/*******************************************************************/
/**
 * NodeMCU WiFi Access Point
 */
#include <ESP8266WiFi.h>
/*******************************************************************/

/*******************************************************************/
/**
 * Web Sockets
 *
 * https://www.thethingsnetwork.org/docs/devices/bytes.html
 * https://www.reddit.com/r/arduino/comments/2pf8uw/best_method_to_extract_floats_from_strings/
 * http://www.cplusplus.com/reference/cstdlib/strtof/
 * https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html
 * 
 */
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);
/*******************************************************************/

/*******************************************************************/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *ctrlData, size_t length) // num is the current client/connection number/ID.  ctrlData is the received data. This is a pointer not a char or char array.
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
        rslt = radio.write(ctrlData, length);
        Serial.print("Data Sent ");
        Serial.printf("%s", ctrlData);
        if (rslt)
        { 
            if (radio.isAckPayloadAvailable())
            {
                radio.read(&imuData, sizeof(imuData));
                newData = true;
            }
            else
            {
                Serial.println("  Acknowledge but no data ");
            }
        }
        else
        {
            Serial.println("Tx Failed");
        }
        //webSocket.sendTXT(num, ctrlData,sizeof(ctrlData),false);
        break;
    }
}
/*******************************************************************/
void setup()
{
    /*******************************************************************/
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);

    // Radio Communication (nRF24L01+LA+PNA)
    Serial.println("SimpleTxAckPayload Starting");
    radio.begin();
    radio.setDataRate(RF24_250KBPS);
    radio.enableAckPayload();
    radio.setRetries(3, 5); // delay, count  // 5 gives a 1500 µsec delay which is needed for a 32 byte ackPayload
    radio.openWritingPipe(address);

    /*******************************************************************/
    // Starting Access Point (NodeMCU)
    Serial.println("Starting AP...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("NodeMCU Access Point", "12345678");
    Serial.println("AP Started!");

    /*******************************************************************/
    // Start Websocket Server
    Serial.println("Starting Websocket...");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Serial.println("Websocket Started!");

    /*******************************************************************/
    // MDSN Server
    if (MDNS.begin("simcontrol"))
    {
        Serial.println("MDNS responder started!");
    }
    Serial.println("Starting Server...");
    //handle index
    server.on(
        "/", []()
        { server.send(200, "text/plain", "You are connected"); });
    server.begin();
    Serial.println("Server Started!");
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);
    /*******************************************************************/
}


/*******************************************************************/
void showData()
{
    if (newData == true)
    {
        Serial.print("  Acknowledge data ");
        Serial.print(imuData[0]);
        Serial.print(", ");
        Serial.print(imuData[1]);
        Serial.print(", ");
        Serial.print(imuData[2]);
        Serial.print(", ");
        Serial.println(imuData[3]);
        /*Serial.print(", ");
        Serial.println(imuData[4]);
        Serial.print(", ");
        Serial.print(imuData[5]);
        Serial.print(", ");
        Serial.print(imuData[6]);
        Serial.print(", ");
        Serial.println(imuData[7]);
        Serial.print(", ");
        Serial.print(imuData[8]);
        Serial.print(", ");
        Serial.print(imuData[9]);
        Serial.print(", ");
        Serial.print(imuData[10]);
        Serial.print(", ");
        Serial.print(imuData[11]);
        Serial.print(", ");
        Serial.print(imuData[12]);
        Serial.print(", ");
        Serial.print(imuData[13]);
        Serial.print(", ");
        Serial.print(imuData[14]);
        Serial.print(", ");
        Serial.print(imuData[15]);
        Serial.print(", ");
        Serial.print(imuData[16]);
        Serial.print(", ");
        Serial.print(imuData[17]);
        Serial.print(", ");
        Serial.print(imuData[18]);
        Serial.print(", ");
        Serial.println(imuData[19]);*/
        Serial.println();
        newData = false;
    }
}
/*******************************************************************/

/*******************************************************************/
void loop()
{
    webSocket.loop();
    server.handleClient();
    showData();
}