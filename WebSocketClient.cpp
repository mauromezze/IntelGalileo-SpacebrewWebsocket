/*
 WebsocketClient, a websocket client for Intel Galileo (Arduino compatible board)
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

/*
 * Base64 Encoding Only Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 * Base64 Encoding Only - Portions Copyright (c) 1995 by International Business Machines, Inc.
 *
 * International Business Machines, Inc. (hereinafter called IBM) grants
 * permission under its copyrights to use, copy, modify, and distribute this
 * Software with or without fee, provided that the above copyright notice and
 * all paragraphs of this notice appear in all copies, and that the name of IBM
 * not be used in connection with the marketing of any product incorporating
 * the Software or modifications thereof, without specific, written prior
 * permission.
 *
 * To the extent it has a right to do so, IBM grants an immunity from suit
 * under its patents, if any, for the use, sale or manufacture of products to
 * the extent that such products are used for performing Domain Name System
 * dynamic updates in TCP/IP networks by means of the Software.  No immunity is
 * granted for any product per se or for any other function of any product.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", AND IBM DISCLAIMS ALL WARRANTIES,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL IBM BE LIABLE FOR ANY SPECIAL,
 * DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE, EVEN
 * IF IBM IS APPRISED OF THE POSSIBILITY OF SUCH DAMAGES.
 */

#include <WebSocketClient.h>
#include <stdlib.h>
#include <stdarg.h>

//*// prog_char --> char
char clientHandshakeLine1a[] PROGMEM = "GET ";
char clientHandshakeLine1b[] PROGMEM = " HTTP/1.1";
char clientHandshakeLine2[] PROGMEM = "Upgrade: WebSocket";
char clientHandshakeLine3[] PROGMEM = "Connection: Upgrade";
char clientHandshakeLine4[] PROGMEM = "Host: ";
char clientHandshakeLine5[] PROGMEM = "Sec-WebSocket-Origin: ArduinoWebSocketClient";
char clientHandshakeLine6[] PROGMEM = "Sec-WebSocket-Version: 13";
char clientHandshakeLine7[] PROGMEM = "Sec-WebSocket-Key: ";
char clientHandshakeLine8[] PROGMEM = "Sec-WebSocket-Protocol: ";
char serverHandshake[] PROGMEM = "HTTP/1.1 101";

PROGMEM const char *WebSocketClientStringTable[] =
{
  clientHandshakeLine1a,
  clientHandshakeLine1b,
  clientHandshakeLine2,
  clientHandshakeLine3,
  clientHandshakeLine4,
  clientHandshakeLine5,
  clientHandshakeLine6,
  clientHandshakeLine7,
  clientHandshakeLine8,
  serverHandshake
};

void WebSocketClient::getStringTableItem(char* buffer, int index) {
  //*// strcpy_P(buffer, (char*)pgm_read_word(&(WebSocketClientStringTable[index])));
  strcpy(buffer, WebSocketClientStringTable[index]);
}

void WebSocketClient::connect(char hostname[], int port, char protocol[], char path[]) {
#ifdef DEBUG2
  Serial.print("WebSocketClient::connect()");
  Serial.print(" hostname: "); Serial.print(hostname);
  Serial.print(" port: "); Serial.println(port);
#endif
  _hostname = hostname;
  _port = port;
  _protocol = protocol;
  _path = path;
  _retryTimeout = millis();
  _canConnect = true;
}

void WebSocketClient::reconnect() {
#ifdef DEBUG2
  Serial.println("WebSocketClient::reconnect()");
#endif
  bool result = false;
  if(_client.connect(_hostname, _port)) {
    sendHandshake(_hostname, _path, _protocol);
    result = readHandshake();
  }
#ifdef DEBUG2
  Serial.print("WebSocketClient::reconnect() result="); Serial.println(result);
#endif

  if(!result) {
#ifdef DEBUG
    Serial.println("DEBUG: Connection Failed!");
#endif
    if(_onError != NULL) {
      _onError(*this, "Connection Failed!");
    }
    _client.stop();
  } else {
    if(_onOpen != NULL) {
      _reconnecting = false; //*// added
      _onOpen(*this);
    }
  }
}

bool WebSocketClient::connected() {
  return _client.connected();
}

void WebSocketClient::disconnect() {
  _client.stop();
}

byte WebSocketClient::nextByte() {
  while(_client.available() == 0);
  byte b = _client.read();

#ifdef DEBUG
  if(b < 0) { Serial.println("DEBUG: Internal Error in Ethernet Client Library (-1 returned where >= 0 expected)"); }
#endif

  return b;
}

void WebSocketClient::monitor() {
  //*//if(!_canConnect) { return; }   // NOTE: these checks are to be revised
  //*//if(_reconnecting) { return; }

  if(!connected() && millis() > _retryTimeout) {
    _retryTimeout = millis() + RETRY_TIMEOUT;
    _reconnecting = true;
    reconnect();
    _reconnecting = false;
    return;
  }

  if(_client.available() > 0) { //*// it was > 2: NOTE: still to check / enhance
    byte hdr = nextByte();
    bool fin = hdr & 0x80;
#ifdef TRACE
    Serial.print("WebSocketClient::monitor() - fin="); Serial.println(fin);
#endif

    int opCode = hdr & 0x0F;
#ifdef TRACE
    Serial.print("WebSocketClient::monitor() - op="); Serial.println(opCode);
#endif

    hdr = nextByte();
    bool mask = hdr & 0x80;
    int len = hdr & 0x7F;
    if(len == 126) {
      len = nextByte();
      len <<= 8;
      len += nextByte();
    } else if (len == 127) {
      len = nextByte();
      for(int i = 0; i < 7; i++) { // NOTE: This may not be correct.  RFC 6455 defines network byte order(??). (section 5.2)
        len <<= 8;
        len += nextByte();
      }
    }
#ifdef TRACE
    Serial.print("WebSocketClient::monitor() - len="); Serial.println(len);
#endif

    if(mask) { // skipping 4 bytes for now.
      for(int i = 0; i < 4; i++) { nextByte(); }
    }

    if(mask) {
#ifdef DEBUG
      Serial.println("DEBUG: Masking not yet supported (RFC 6455 section 5.3)");
#endif

      if(_onError != NULL) { _onError(*this, "Masking not supported"); }
      free(_packet);
      return;
    }

    if(!fin) {
      if(_packet == NULL) {
        _packet = (char*) malloc(len);
        for(int i = 0; i < len; i++) { _packet[i] = nextByte(); }
        _packetLength = len;
        _opCode = opCode;
      } else {
        int copyLen = _packetLength;
        _packetLength += len;
        char *temp = _packet;
        _packet = (char*)malloc(_packetLength);
        for(int i = 0; i < _packetLength; i++) {
          if(i < copyLen) {
            _packet[i] = temp[i];
          } else {
            _packet[i] = nextByte();
          }
        }
        free(temp);
      }
      return;
    }

    if(_packet == NULL) {
      _packet = (char*) malloc(len + 1);
      for(int i = 0; i < len; i++) { _packet[i] = nextByte(); }
      _packet[len] = 0x0;
    } else {
      int copyLen = _packetLength;
      _packetLength += len;
      char *temp = _packet;
      _packet = (char*) malloc(_packetLength + 1);
      for(int i = 0; i < _packetLength; i++) {
        if(i < copyLen) {
          _packet[i] = temp[i];
        } else {
          _packet[i] = nextByte();
        }
      }
      _packet[_packetLength] = 0x0;
      free(temp);
    }
    
    if(opCode == 0 && _opCode > 0) {
      opCode = _opCode;
      _opCode = 0;
    }

    switch(opCode) {
      case 0x00:
#ifdef DEBUG
        Serial.println("DEBUG: Unexpected Continuation OpCode");
#endif
        break;
        
      case 0x01:
#ifdef DEBUG
        Serial.println("DEBUG: onMessage");
#endif
        if(_onMessage != NULL) { _onMessage(*this, _packet); }
        break;
        
      case 0x02:
#ifdef DEBUG
        Serial.println("DEBUG: Binary messages not yet supported (RFC 6455 section 5.6)");
#endif
        if(_onError != NULL) { _onError(*this, "Binary Messages not supported"); }
        break;
        
      case 0x09:
#ifdef DEBUG
        Serial.println("DEBUG: onPing");
#endif
        _client.write(0x8A);
        _client.write(byte(0x00));
        break;
        
      case 0x0A:
#ifdef DEBUG
        Serial.println("DEBUG: onPong");
#endif
        break;
        
      case 0x08:
        unsigned int code = ((byte)_packet[0] << 8) + (byte)_packet[1];
#ifdef DEBUG
        Serial.println("DEBUG: onClose");
#endif
        if(_onClose != NULL) { _onClose(*this, code, (_packet + 2)); }
        _client.stop();
        break;
    }

    free(_packet);
    _packet = NULL;
  }
}

void WebSocketClient::onMessage(OnMessage fn) { _onMessage = fn; }

void WebSocketClient::onOpen(OnOpen fn) { _onOpen = fn; }

void WebSocketClient::onClose(OnClose fn) { _onClose = fn; }

void WebSocketClient::onError(OnError fn) { _onError = fn; }

void WebSocketClient::sendHandshake(char* hostname, char* path, char* protocol) {
#ifdef DEBUG2
  Serial.println("WebSocketClient::sendHandshake()");
#endif
  char buffer[45];

  getStringTableItem(buffer, 0); _client.print(buffer); _client.print(path);
#ifdef HANDSHAKE
  Serial.print("Handshake 1: "); Serial.print(buffer); Serial.print(path);
#endif

  getStringTableItem(buffer, 1); _client.println(buffer);
#ifdef HANDSHAKE
  Serial.println(buffer);
#endif

  getStringTableItem(buffer, 2); _client.println(buffer);
#ifdef HANDSHAKE
  Serial.print("Handshake 2: "); Serial.println(buffer);
#endif

  getStringTableItem(buffer, 3); _client.println(buffer);
#ifdef HANDSHAKE
  Serial.print("Handshake 3: "); Serial.println(buffer);
#endif

  getStringTableItem(buffer, 4); _client.print(buffer); _client.println(hostname);
#ifdef HANDSHAKE
  Serial.print("Handshake 4: "); Serial.print(buffer); Serial.println(hostname);
#endif

  getStringTableItem(buffer, 5); _client.println(buffer);
#ifdef HANDSHAKE
  Serial.print("Handshake 5: "); Serial.println(buffer);
#endif

  getStringTableItem(buffer, 6); _client.println(buffer);
#ifdef HANDSHAKE
  Serial.print("Handshake 6: "); Serial.println(buffer);
#endif

  getStringTableItem(buffer, 7); _client.print(buffer);
#ifdef HANDSHAKE
  Serial.print("Handshake 7: "); Serial.print(buffer);
#endif

  generateHash(buffer, 45); _client.println(buffer);
#ifdef HANDSHAKE
  Serial.println(buffer);
#endif

  getStringTableItem(buffer, 8); _client.print(buffer); _client.println(protocol);
#ifdef HANDSHAKE
  Serial.print("Handshake 8: "); Serial.print(buffer); Serial.println(protocol);
#endif

  _client.println();
#ifdef HANDSHAKE
  Serial.println();
#endif
}

bool WebSocketClient::readHandshake() {
#ifdef DEBUG2
  Serial.println("WebSocketClient::readHandshake()");
#endif
  bool result = false;
  char line[128];
  int maxAttempts = 300, attempts = 0;
  char response[12];
  getStringTableItem(response, 9);

  while(_client.available() == 0 && attempts < maxAttempts) {
    delay(100);
    attempts++;
  }

  while(true) {
    readLine(line);
#ifdef HANDSHAKE
    Serial.print("Read Handshake: "); Serial.println(line);
#endif

    if(strcmp(line, "") == 0) { break; }
    if(strncmp(line, response, 12) == 0) { result = true; }
  }

  if(!result) {
#ifdef DEBUG
    Serial.println("Handshake Failed! Terminating");
#endif
    _client.stop();
  }
  return result;
}

void WebSocketClient::readLine(char* buffer) {
  char character;
  int i = 0;
  while(_client.available() > 0 && (character = _client.read()) != '\n') {
    if(character != '\r' && character != -1) {
      buffer[i++] = character;
    }
  }
  buffer[i] = 0x0;
}

bool WebSocketClient::send(char* message) {
#ifdef DEBUG2
  Serial.print("WebSocketClient::send() - canConnect="); Serial.print(_canConnect);
  Serial.print(" - reconnecting="); Serial.println(_reconnecting);
  if(!_canConnect || _reconnecting) { return false; }
#endif
  
#ifdef DEBUG2
  Serial.print("WebSocketClient::send() - message="); Serial.println(message);
#endif

  int len = strlen(message);
  _client.write(0x81);
  if(len > 125) {
    _client.write(0xFE);
    _client.write(byte(len >> 8));
    _client.write(byte(len & 0xFF));
  } else {
    _client.write(0x80 | byte(len));
  }
  for(int i = 0; i < 4; i++) {
    _client.write((byte)0x00); // use 0x00 for mask bytes which is effectively a NOOP
  }
  _client.print(message);
  return true;
}

void WebSocketClient::generateHash(char buffer[], size_t bufferlen) {
  byte bytes[16];
  for(int i = 0; i < 16; i++) {
    //*//bytes[i] = random(255);
    bytes[i] = 255 * random();
  }
  base64Encode(bytes, 16, buffer, bufferlen);
}

size_t WebSocketClient::base64Encode(byte* src, size_t srclength, char* target, size_t targsize) {
  size_t datalength = 0;
  char input[3];
  char output[4];
  size_t i;

  while (2 < srclength) {
    input[0] = *src++;
    input[1] = *src++;
    input[2] = *src++;
    srclength -= 3;

    output[0] = input[0] >> 2;
    output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
    output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);
    output[3] = input[2] & 0x3f;

    if(datalength + 4 > targsize) { return (-1); }

    target[datalength++] = b64Alphabet[output[0]];
    target[datalength++] = b64Alphabet[output[1]];
    target[datalength++] = b64Alphabet[output[2]];
    target[datalength++] = b64Alphabet[output[3]];
  }

  // Padding
  if(0 != srclength) {
    input[0] = input[1] = input[2] = '\0';
    for (i = 0; i < srclength; i++) { input[i] = *src++; }

    output[0] = input[0] >> 2;
    output[1] = ((input[0] & 0x03) << 4) + (input[1] >> 4);
    output[2] = ((input[1] & 0x0f) << 2) + (input[2] >> 6);

    if(datalength + 4 > targsize) { return (-1); }

    target[datalength++] = b64Alphabet[output[0]];
    target[datalength++] = b64Alphabet[output[1]];
    if(srclength == 1) {
      target[datalength++] = '=';
    } else {
      target[datalength++] = b64Alphabet[output[2]];
    }
    target[datalength++] = '=';
  }
  if(datalength >= targsize) { return (-1); }
  target[datalength] = '\0';
  return (datalength);
}
