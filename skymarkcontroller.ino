// tis is the deployed code and it is workin fine. in this code whether the server is starting first or client is starting first it will work
#include <SPI.h>
#include <Ethernet.h>

//car entry entry_gate1
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01};
IPAddress ip(192, 168, 3, 16);

// bike entry For entry_gate2
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x02};
//IPAddress ip(192, 168, 3, 15);

// car exit For exit_gate1
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03};
//IPAddress ip(192, 168, 3, 17);

// bike exit  exit_gate2
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x04};
//IPAddress ip(192, 168, 3, 18);

IPAddress serverIp(192, 168, 3, 36);
EthernetClient client;

const int relayPin = 9;

void setup() {
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
}

void loop() {
  if (client.connect(serverIp, 5000)) {
    client.println("GET /check_boomsig?gate_id=entry_gate2 HTTP/1.1");
    client.println("Host: 192.168.3.36");
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      if (client.available()) {
        String line = client.readStringUntil('\n');
        Serial.println(line);

        if (line.indexOf("|OPENEN%") >= 0) {
          Serial.println("Opening gate...");
          digitalWrite(relayPin, LOW);
          delay(500);
          digitalWrite(relayPin, HIGH);
        }
      }
    }
    client.stop();
  } else {
    Serial.println("Connection to server failed");
  }
  delay(5000);
}



//Ex-2 this is the deployed code and it is working but the problem is the server should be start first then the arduino client will be connected. but when power fails then it will not send the data to the server
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoHttpClient.h>
#define relayPin 9

// MAC and IP for entry_gate1
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01};
IPAddress ip(192, 168, 1, 157);

// For entry_gate2
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x02};
// IPAddress ip(192, 168, 1, 158);

// For exit_gate1
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03};
// IPAddress ip(192, 168, 1, 159);

// For exit_gate2
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x04};
// IPAddress ip(192, 168, 1, 160);

IPAddress server(192, 168, 1, 100);
EthernetClient client;
HttpClient http(client, server, 5000);

const char* gateID = "entry_gate1";
// const char* gateID = "entry_gate2";
// const char* gateID = "exit_gate1";
// const char* gateID = "exit_gate2";

unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 3000;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Ethernet.begin(mac, ip);
  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
}

void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastRequestTime >= requestInterval) {
    lastRequestTime = currentTime;
    checkBoomSignal();
  }
}

void checkBoomSignal() {
  String path = "/check_boomsig?gate_id=" + String(gateID);
  Serial.print("Sending request to: ");
  Serial.println(path);

  http.get(path);

  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Response Code: ");
  Serial.println(statusCode);
  Serial.print("Response Body: ");
  Serial.println(response);

  if (statusCode == 200 && response.indexOf("|OPENEN%") >= 0) {
    triggerBarrier();
  }
  http.stop();
}

void triggerBarrier() {
  Serial.println("Barrier is opening...");
  digitalWrite(relayPin, LOW);  // Activate relay (assuming LOW is ON)
  delay(500);
  digitalWrite(relayPin, HIGH); // Deactivate relay
  delay(500);
}

//EX-3 i want to remove the polling the request 
#include <SPI.h>
#include <Ethernet.h>
#include <WebSocketsClient.h>

#define relayPin 9

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01};
IPAddress ip(192, 168, 1, 157);

const char* websocket_host = "192.168.1.100";
const uint16_t websocket_port = 5000;
const char* gateID = "entry_gate1";

EthernetClient ethClient;
WebSocketsClient webSocket;

void setup() {
  Serial.begin(9600);
  while (!Serial) { ; }

  Ethernet.begin(mac, ip);
  delay(1000);

  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  webSocket.begin(websocket_host, websocket_port, "/ws/" + String(gateID));
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.println("Connected to WebSocket server");
      break;

    case WStype_DISCONNECTED:
      Serial.println("Disconnected from WebSocket server");
      break;

    case WStype_TEXT:
      Serial.print("Message received: ");
      Serial.println((char*)payload);
      if (String((char*)payload).indexOf("|OPENEN%") >= 0) {
        triggerBarrier();
      }
      break;

    default:
      break;
  }
}

void triggerBarrier() {
  Serial.println("Barrier is opening...");
  digitalWrite(relayPin, LOW);
  delay(500);
  digitalWrite(relayPin, HIGH);
  delay(500);
}

