This code connects the NodeMCU with the JoyStick Master on Android
The code for JoyStick Master Android app can be found under the JoyStick Master Repository. 
The JoyStick Master provides the joystick commands via websockets to the NodeMCU. 
In settings, the WiFi settings initiate the WiFi from the NodeMCU to which the Android is to be connected. 
Once the WiFi settings are initialized, the NodeMCU is initialized as a WebSocket Server. 
After this the RF24 module is initialized and it can start sending and receiving data to and frrom the Teensy 4.1 Slave on the Model Aircraft. 
 