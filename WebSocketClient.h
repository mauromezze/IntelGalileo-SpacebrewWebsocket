/*
 WebsocketClient, a websocket client for Arduino
 Copyright 2011 Kevin Rohling
 Copyright 2012 Ian Moore
 Copyright 2014 Luca Mari
 http://kevinrohling.com
 http://www.incamoon.com
 http://www.liuc.it/persone/lmari
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

//#define DEBUG // uncomment to turn on debugging
//#define DEBUG2 // uncomment to turn on deep debugging //*//
//#define HANDSHAKE // uncomment to print out the sent and received handshake messages
//#define TRACE // uncomment to support TRACE level debugging of wire protocol

#define RETRY_TIMEOUT 3000

#include <stdlib.h>
#include <Ethernet.h>
#include "Arduino.h"

class WebSocketClient {
public:
  typedef void (*OnMessage)(WebSocketClient client, char* message);
  typedef void (*OnOpen)(WebSocketClient client);
  typedef void (*OnClose)(WebSocketClient client, int code, char* message);
  typedef void (*OnError)(WebSocketClient client, char* message);
  void setDebug(Stream *debug);
  void connect(char hostname[], int port = 80, char protocol[] = NULL, char path[] = "/");
  bool connected();
  void disconnect();
  void monitor();
  void onOpen(OnOpen function);
  void onClose(OnClose function);
  void onMessage(OnMessage function);
  void onError(OnError function);
  bool send(char* message);
private:
  char* _hostname;
  int _port;
  char* _path;
  char* _protocol;
  bool _canConnect;
  bool _reconnecting;
  unsigned long _retryTimeout;
  void getStringTableItem(char* buffer, int index);
  void reconnect();
  void sendHandshake(char* hostname, char* path, char* protocol);
  EthernetClient _client;
  OnOpen _onOpen;
  OnClose _onClose;
  OnMessage _onMessage;
  OnError _onError;
  char* _packet;
  unsigned int _packetLength;
  byte _opCode;
  bool readHandshake();
  void readLine(char* buffer);
  void generateHash(char* buffer, size_t bufferlen);
  size_t base64Encode(byte* src, size_t srclength, char* target, size_t targetsize);
  byte nextByte();
};

const char b64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif
