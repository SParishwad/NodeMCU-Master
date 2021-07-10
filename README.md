This code connects the NodeMCU with the JoyStick Master on Android
The code for JoyStick Master Android app can be found under the JoyStick Master Repository. 
The JoyStick Master provides the joystick commands via websockets to the NodeMCU. 
In settings, the WiFi settings initiate the WiFi from the NodeMCU to which the Android is to be connected. 
Once the WiFi settings are initialized, the NodeMCU is initialized as a WebSocket Server. 
After this the RF24 module is initialized and it can start sending and receiving data to and frrom the Teensy 4.1 Slave on the Model Aircraft. 
 
 # NodeMCU as a master for controlling a Model Airplane

This code is a part of a larger project. Ideally it can be used in combination with the "Remote Controller" Android App 
to controll any remote controlled vehicle. 

The Code can be split into the following parts: 
1. NodeMCU WiFi Access Point
2. WebSockets Communication with Android App
3. Radio Communication

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

The following piece of code makes 
```
#include <WebSocketsServer.h>

ESP8266WebServer server(80);       // Create a webserver object that listens for HTTP request on port 80
WebSocketsServer webSocket(81);    // create a websocket server on port 81
```

## Radio Communication
The  nRF24L01+LA+PNA module is 



## nrf24L01


## Libraries used