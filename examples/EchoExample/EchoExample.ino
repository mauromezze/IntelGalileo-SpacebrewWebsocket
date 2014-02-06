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
