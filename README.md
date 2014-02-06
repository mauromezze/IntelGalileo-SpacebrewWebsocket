# ArduinoWebsocketClient, an Arduino client for connecting and messaging with Websockets

Websockets currently provide a simple and lightweight way to send and receive messages from web browsers.  This project was developed to extend this capability to embedded devices (Arduinos).  It is my hope that allowing devices to easily send information about themselves as well as respond to messages received from hosted services will result in some interesting applications.

## WebsocketClient for Intel Galileo and specifically for its usage with Spacebrew
This version of WebsocketClient has been adapted by Luca Mari, 2014, to Intel Galileo (Arduino compatible board) and specifically for its usage with Spacebrew.

Based on ArduinoWebsocketClient by Kevin Rohling and Ian Moore, please refer to the following
Blog: [World Domination Using Arduinos And Websockets](http://kevinrohling.wordpress.com/2011/09/14/world-domination-using-arduinos-and-websockets)
Blog: [Smart Phone Powered Home Automation using Websockets and Amazon EC2](http://www.incamoon.com)

The received situation was of two incompatible versions of the Arduino WebsocketClient library:
-- https://github.com/krohling/ArduinoWebsocketClient (implementing the online websocket protocol)
-- https://github.com/labatrockwell/ArduinoWebsocketClient (oriented to Spacebrew) neither of them supporting Galileo, an Intel SoC Pentium-based board.
They have been revised, modified, and integrated, so that this version runs on Galileo and
works for both the connection to a server such as echo.websocket.org (FOR_SPACEBREW not defined) and Spacebrew (FOR_SPACEBREW defined).
This version includes extended tracing facilities for debugging (see WebSocketClient.h).
The main changes with respect to the previous versions are marked by slash-slash-star-slash-slash.

## Caveats

This library doesn't support every inch of the Websocket spec, most notably the use of a Sec-Websocket-Key.  Also, because Arduino doesn't support SSL, this library also doesn't support the use of Websockets over https.  If you're interested in learning more about the Websocket spec I recommend checking out the [Wikipedia Page](http://en.wikipedia.org/wiki/WebSocket).  Now that I've got that out of the way, I've been able to successfully use this to connect to several hosted Websocket services, including: [echo.websocket.org](http://websocket.org/echo.html) and [pusherapp.com](http://pusherapp.com).

UPDATE: [RFC 6544](http://tools.ietf.org/html/rfc6455) support is improving.  Sec-Websocket-Key + Sec-Websocket-Protocol are now supported.  Also, the client will re-establish lost connections.

## Installation instructions

Once you've cloned this repo locally, copy the ArduinoWebsocketClient directory into your Arduino Sketchbook directory under Libraries then restart the Arduino IDE so that it notices the new library.  Now, under File\Examples you should see ArduinoWebsocketClient.  To use the library in your app, select Sketch\Import Library\ArduinoWebsocketClient.

## How To Use This Library

```
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char server[] = "echo.websocket.org";
WebSocketClient client;

void setup() {
  Serial.begin(9600);
  Ethernet.begin(mac);
  client.connect(server);
  client.onMessage(onMessage);
  client.send("Hello World!");
}

void loop() {
  client.monitor();
}

void onMessage(WebSocketClient client, char* message) {
  Serial.print("Received: ");
  Serial.println(message);
}
```

## Examples

The example included with this library, EchoExample, will connect to echo.websocket.org, which hosts a service that simply echos any messages that you send it via Websocket.  This example sends the message "Hello World!".  If the example runs correctly, the Arduino will receive this same message back over the Websocket and print it via Serial.println.