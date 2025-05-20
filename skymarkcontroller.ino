// tis is the deployed code and it is workin fine. in this code whether the server is starting first or client is starting first it will work
#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 157);
IPAddress serverIp(192, 168, 1, 50); // IP where Flask runs
EthernetClient client;

const int relayPin = 9;

void setup() {
  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);  // Relay off (active LOW)
}

void loop() {
  if (client.connect(serverIp, 5000)) { // Flask running on port 8000
    client.println("GET /check_boomsig?gate_id=entry_gate1 HTTP/1.1");
    client.println("Host: 192.168.1.100");
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
#include <Ethernet.h>

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

