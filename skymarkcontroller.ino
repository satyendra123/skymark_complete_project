#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#define relayPin 9

// MAC and IP for entry_gate1
//byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01};
//IPAddress ip(192, 168, 1, 157);

// For entry_gate2
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x02};
IPAddress ip(192, 168, 1, 158);

// For exit_gate1
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x03};
// IPAddress ip(192, 168, 1, 159);

// For exit_gate2
// byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x04};
// IPAddress ip(192, 168, 1, 160);

IPAddress server(192, 168, 1, 50);
EthernetClient client;
HttpClient http(client, server, 5000);

//const char* gateID = "entry_gate1";
const char* gateID = "entry_gate2";
// const char* gateID = "exit_gate1";
// const char* gateID = "exit_gate2";

unsigned long lastRequestTime = 0;
const unsigned long requestInterval = 2000;

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

  if (statusCode == 200 && response.indexOf("|OPENEN%") >= 0) {
    triggerBarrier();
  }
  http.stop();
}

void triggerBarrier() {
  Serial.println("Barrier is opening...");
  digitalWrite(relayPin, LOW);
  delay(500);
  digitalWrite(relayPin, HIGH);
  delay(500);
}
