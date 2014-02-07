# ArduinoWebsocketClient, an Arduino client for connecting and messaging with Websockets
Websockets currently provide a simple and lightweight way to send and receive messages from web browsers.  This project was developed to extend this capability to embedded devices (Arduinos).  It is my hope that allowing devices to easily send information about themselves as well as respond to messages received from hosted services will result in some interesting applications.

## WebsocketClient for Intel Galileo and specifically for its usage with Spacebrew
This version of WebsocketClient has been adapted by Luca Mari, 2014, to Intel Galileo (Arduino compatible board) and specifically for its usage with Spacebrew.

Based on ArduinoWebsocketClient by Kevin Rohling and Ian Moore, please refer to the following
Blog: [World Domination Using Arduinos And Websockets](http://kevinrohling.wordpress.com/2011/09/14/world-domination-using-arduinos-and-websockets)
Blog: [Smart Phone Powered Home Automation using Websockets and Amazon EC2](http://www.incamoon.com)

The received situation was of a version of the Arduino WebsocketClient library:
    https://github.com/labatrockwell/ArduinoWebsocketClient (oriented to Spacebrew)
adapted from:
    https://github.com/krohling/ArduinoWebsocketClient (implementing the online websocket protocol)
neither of them supporting Galileo, an Intel SoC Pentium-based board.
It has been revised, modified, and integrated, so that this version runs on Galileo and works for both the connection to a server such as echo.websocket.org and Spacebrew.
This version includes extended tracing facilities for debugging (see WebSocketClient.h).
The main changes with respect to the previous versions are marked by slash-slash-star-slash-slash.

## Caveats
This library doesn't support every inch of the Websocket spec, most notably the use of a Sec-Websocket-Key.  Also, because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html) and [pusherapp.com](http://pusherapp.com).

UPDATE: [RFC 6544](http://tools.ietf.org/html/rfc6455) support is improving.  Sec-Websocket-Key + Sec-Websocket-Protocol are now supported.  Also, the client will re-establish lost connections.

## Installation instructions
Once you've cloned this repo locally, copy the ArduinoWebsocketClient directory into your Arduino Sketchbook directory under Libraries then restart the Arduino IDE so that it notices the new library.  Now, under File\Examples you should see ArduinoWebsocketClient.  To use the library in your app, select Sketch\Import Library\ArduinoWebsocketClient.

## How To Use This Library: some examples
###First example 
A sketch that connects to the server echo.websocket.org, send a text, and receives the reply.

```
#include "Arduino.h"
#include <Ethernet.h>
#include <SPI.h>
#include <WebSocketClient.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "echo.websocket.org";
WebSocketClient client;

void setup() {
  Serial.begin(9600);
  Serial.println("EXAMPLE: setup()");
  Ethernet.begin(mac);
  client.connect(server);
  client.onOpen(onOpen);
  client.onMessage(onMessage);
  client.onError(onError);
  client.send("Hello World!");
}

void loop() {
  client.monitor();
}

void onOpen(WebSocketClient client) {
  Serial.println("EXAMPLE: onOpen()");
}

void onMessage(WebSocketClient client, char* message) {
  Serial.println("EXAMPLE: onMessage()");
  Serial.print("Received: "); Serial.println(message);
}

void onError(WebSocketClient client, char* message) {
  Serial.println("EXAMPLE: onError()");
  Serial.print("ERROR: "); Serial.println(message);
}
```

**************************************************
###Second example
A Spacebrew client, tested on Intel Galileo, operating as subscriber that does not require any additional hardware.
It just turns the internal led on and off.


```
#include <SPI.h>
#include <Spacebrew.h>
#include <Ethernet.h>
#include <WebSocketClient.h>

char *spacebrewServerAddress = "10.0.16.40"; //change it appropriately
char *spacebrewClientName = "Blink LED";
char *spacebrewClientDesc = "A Boolean subscriber with Arduino";
char *spacebrewPublisherName = "MyLed";

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
Spacebrew spacebrewConnection;
const int ledPin = 13;

void setup() {
  Ethernet.begin(mac);
  pinMode(ledPin, OUTPUT);

  spacebrewConnection.onOpen(onOpen);
  spacebrewConnection.onBooleanMessage(onBooleanMessage);
  spacebrewConnection.addSubscribe(spacebrewPublisherName, SB_BOOLEAN);

  spacebrewConnection.connect(spacebrewServerAddress, spacebrewClientName, spacebrewClientDesc);
}

void loop() {
  spacebrewConnection.monitor();
}

void onOpen() {
  Serial.println("EXAMPLE: onOpen()");  
}

void onBooleanMessage(char *name, bool value) {
  int ledValue;
  if(value) {
    ledValue = HIGH;
  } else {
    ledValue = LOW;
  }
  digitalWrite(ledPin, ledValue);
  Serial.print("EXAMPLE: onBooleanMessage(): ");
  Serial.println(ledValue);
}
```

**************************************************
###Third example
A Spacebrew client, tested on Intel Galileo, operating as publisher that does not require any additional hardware.
It just sends an alternate on/off signal.

```
#include <SPI.h>
#include <Spacebrew.h>
#include <Ethernet.h>
#include <WebSocketClient.h>

char *spacebrewServerAddress = "10.0.16.40"; //change it appropriately
char *spacebrewClientName = "A Switcher";
char *spacebrewClientDesc = "A Boolean publisher with Arduino";
char *spacebrewPublisherName = "MySwitcher";

uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
Spacebrew spacebrewConnection;

bool x;

void setup() {
  Ethernet.begin(mac);
  spacebrewConnection.addPublish(spacebrewPublisherName, SB_BOOLEAN);
  spacebrewConnection.connect(spacebrewServerAddress, spacebrewClientName, spacebrewClientDesc);
}

void loop() {
  spacebrewConnection.monitor();
  x = 1 - x;
  spacebrewConnection.send(spacebrewPublisherName, x);
  Serial.println(x);
  delay(1000);
}
```
