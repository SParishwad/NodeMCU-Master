 # NodeMCU as a master for controlling a Model Airplane

This code is a part of a larger project. Ideally it can be used in combination with the "Remote Controller" Android App 
to controll any remote controlled vehicle. 

The Code can be split into the following parts: 
1. NodeMCU WiFi Access Point
2. WebSockets Communication with Android App
3. Radio Communication
4. IMU Data reception and transmission to the android 

## NodeMCU WiFi Access Point
NodeMCU can be used to create a WiFi Network to which the Android has to be connected to, to be able to establish a WebSocket connection. 
This is done by importing a `ESP8266WiFi` Library and the `WiFi.softAP(ssid, password);` method. 

```
#include <ESP8266WiFi.h>

void setup(){
  Serial.println("Starting AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("NodeMCU Access Point", "12345678");
  Serial.println("AP Started!");
}
```

## WebSockets
The Remote Controller Android App uses WebSockets to communicate with an android device and receives the controller signals from two joysticks.  

The use of WebSockets was essential, in order to continuously send and receive the control signals between a client and server. Websockets are a full duplex, low latency, low overhead protocol that keeps a TCP Connection open until it is closed by the App.

The Plane also has two IMU Modules which will be used to close a feedback loop for stabilization of the plane directly by the controller on board and to get rid of any challenges which may appear due to the high latency from the long transmission route. 

The WebSocket Library by [Markus Sattler](https://github.com/Links2004/arduinoWebSockets.git) is used. 

In this part, the library is included and a webSocket instance is initialized on Port 81. In the setup, the webSocket server is started.
```
#include <WebSocketsServer.h>

ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81

void setup(){
  // Start Websocket Server
  Serial.println("Starting Websocket...");
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  Serial.println("Websocket Started!");
}
```

In the main loop, a call to the Event Handler is made

```
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
```

Where `WStype_t type` is defined as:

```
typedef enum {
    WStype_ERROR,
    WStype_DISCONNECTED,
    WStype_CONNECTED,
    WStype_TEXT,
    WStype_BIN,
    WStype_FRAGMENT_TEXT_START,
    WStype_FRAGMENT_BIN_START,
    WStype_FRAGMENT,
    WStype_FRAGMENT_FIN,
    WStype_PING,
    WStype_PONG,
} WStype_t;
```
If `WStype_TEXT` data is received from the WebSocket, the data is transmitted via Radio to the Plane in the event handler. 

## Radio Communication
The nRF24L01+LA+PNA module is used to communicate between the Plane and the transmitter on the ground Theoretically it provides upto  800m. 

The RF24 Library by [tmrh20](https://github.com/nRF24/RF24.git) is used. A "[Two-way transmission using the ackPayload concept](https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123/2)" concept is used to transfer data to the plane and receive the IMU data from the Plane on the Android device for storage and analysis. 

```
#include <nRF24L01.h>

void setup(){
  Serial.println("Radio Communnication Starting...");
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00001    // This is checked
  radio.setPALevel(RF24_PA_MIN);
  Serial.println("Radio Communication Started!");
}
```

In the webSocket event handler, the data received from the Android app via webSockets is transmitted to the plane using the 
`radio.write(payload, length);` statement.


### Further Use: 
The code can be modified as to make the NodeMCU react directly to the data from the Android app. 