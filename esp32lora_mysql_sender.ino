#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define ss 16
#define rst 14
#define dio0 26

const char* ssid = "eedlab";
const char* password = "eedlab@1";
const char* serverName = "http://192.168.43.131/submit_data.php"; // Replace with your local IP

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  // Initialize Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) { // Change to your frequency
    Serial.print(".");
    delay(500);
  }
  
  // Change sync word (0xF3) to match the send`er
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    Serial.print("Received packet: '");
    
    // Read packet
    String incoming = "";
    while (LoRa.available()) {
      incoming += (char)LoRa.read();
    }

    // Print incoming message to Serial Monitor
    Serial.print(incoming);
    Serial.println("'");

    // Parse JSON payload (assuming a correct format)
    int tempIndex = incoming.indexOf("temperature");
    int humIndex = incoming.indexOf("humidity");
    if (tempIndex != -1 && humIndex != -1) {
      String tempString = incoming.substring(tempIndex + 13, incoming.indexOf(",", tempIndex));
      String humString = incoming.substring(humIndex + 10, incoming.indexOf("}", humIndex));

      float temperature = tempString.toFloat();
      float humidity = humString.toFloat();

      // Print temperature and humidity values to Serial Monitor
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" C");

      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

      // Send data to the server
      if(WiFi.status() == WL_CONNECTED){ 
        HTTPClient http;

        String serverPath = String(serverName) + "?temperature=" + String(temperature) + "&humidity=" + String(humidity);
        Serial.print("Requesting URL: ");
        Serial.println(serverPath);
        
        http.begin(serverPath.c_str());
        
        int httpResponseCode = http.GET();
        
        if (httpResponseCode > 0) {
          Serial.print("HTTP Response code: ");
          Serial.println(httpResponseCode);
        } else {
          Serial.print("Error code: ");
          Serial.println(httpResponseCode);
          Serial.print("HTTP Error: ");
          Serial.println(http.errorToString(httpResponseCode).c_str());
        }
        // Free resources
        http.end();
      } else {
        Serial.println("WiFi Disconnected");
      }
    } else {
      Serial.println("Failed to parse JSON payload.");
    }
  }
}