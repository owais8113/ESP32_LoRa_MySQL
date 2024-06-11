#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Define pins and constants for LoRa
#define ss 16
#define rst 14
#define dio0 26

// WiFi and MQTT settings
const char* ssid = "eedlab";
const char* password = "eedlab@1";
const char* mqttServer = "test.mosquitto.org"; // Change to your Mosquitto broker IP address
const int mqttPort = 1883; // Default MQTT port
const char* mqttUser = ""; // MQTT username, if required
const char* mqttPassword = ""; // MQTT password, if required
const char* mqttTopic = "sensor_data"; // MQTT topic to publish data

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) {
    Serial.print(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  // Try to parse LoRa packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.println("Received packet!");

    // Read packet into a String
    String receivedPayload = "";
    while (LoRa.available()) {
      receivedPayload += (char)LoRa.read();
    }

    // Publish data to MQTT topic
    client.publish(mqttTopic, receivedPayload.c_str());
    Serial.println("Published data to MQTT broker");
  }
}