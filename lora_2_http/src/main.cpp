#include <Arduino.h>
#include <RadioLib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <ArduinoJson.h>

// LoRa module pins
#define LORA_NSS_PIN    8
#define LORA_RESET_PIN  12
#define LORA_DIO1_PIN   14
#define LORA_BUSY_PIN   13

#define LORA_CLK        9
#define LORA_MISO       11
#define LORA_MOSI       10

// Create an instance of the LoRa module
SX1262 radio = new Module(LORA_NSS_PIN, LORA_DIO1_PIN, LORA_RESET_PIN, LORA_BUSY_PIN);

// OLED display configuration
#define OLED_RESET      21 
#define OLED_SDA        17
#define OLED_SCL        18

U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA, /* reset=*/ OLED_RESET);

// WiFi credentials
const char* ssid = "LC iPhone";
const char* password = "spacetime";

// API endpoint
const char* baseUrl = "http://172.20.10.4:3344";

// Flag to indicate that a packet was received
volatile bool receivedFlag = false;

// Disable interrupt when it's not needed
volatile bool enableInterrupt = true;

// This function is called when a complete packet is received by the module
// IMPORTANT: this function MUST be 'void' type and MUST NOT have any arguments!
void setFlag(void) {
  // Check if the interrupt is enabled
  if(!enableInterrupt) {
    return;
  }

  // We got a packet, set the flag
  receivedFlag = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("LoRa Receiver and HTTP Client");

  // Initialize LoRa
  Serial.print(F("Initializing LoRa ... "));
  
  // Initialize SPI for LoRa
  SPI.begin(LORA_CLK, LORA_MISO, LORA_MOSI, LORA_NSS_PIN);
  
  // LoRa initialization
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true); // Stop execution if LoRa initialization fails
  }

  // Set up LoRa parameters
  radio.setFrequency(915.0);
  radio.setBandwidth(125.0);
  radio.setSpreadingFactor(7);
  radio.setCodingRate(5);

  // Set the function that will be called 
  // when new packet is received
  radio.setDio1Action(setFlag);

  // Start listening for LoRa packets
  Serial.print(F("[SX1262] Starting to listen ... "));
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("success!"));
  } else {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }

  // Initialize OLED display
  display.begin();
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 10, "LoRa Receiver");
  display.drawStr(0, 20, "HTTP Client Ready");
  display.sendBuffer();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  display.clearBuffer();
  display.drawStr(0, 10, "Connecting to WiFi");
  display.sendBuffer();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  display.clearBuffer();
  display.drawStr(0, 10, "Connected to WiFi");
  display.drawStr(0, 20, "Waiting for LoRa");
  display.sendBuffer();
}

void loop() {
  if(receivedFlag) {
    enableInterrupt = false;
    receivedFlag = false;

    String str;
    int state = radio.readData(str);

    // Print raw data
    Serial.print(F("[SX1262] Data:\t\t"));
    Serial.println(str);

    // Print RSSI (Received Signal Strength Indicator)
    Serial.print(F("[SX1262] RSSI:\t\t"));
    Serial.print(radio.getRSSI());
    Serial.println(F(" dBm"));

    // Print SNR (Signal-to-Noise Ratio)
    Serial.print(F("[SX1262] SNR:\t\t"));
    Serial.print(radio.getSNR());
    Serial.println(F(" dB"));

    // Print frequency error
    Serial.print(F("[SX1262] Frequency error:\t"));
    Serial.print(radio.getFrequencyError());
    Serial.println(F(" Hz"));

    // Display received message
    display.clearBuffer();
    display.drawStr(0, 10, "Received LoRa msg:");
    display.drawStr(0, 20, str.c_str());
    display.sendBuffer();

    // Parse the received message
    int firstColon = str.indexOf(':');
    int secondColon = str.indexOf(':', firstColon + 1);
    
    if (firstColon != -1 && secondColon != -1) {
      String action = str.substring(0, firstColon);
      String userId = str.substring(firstColon + 1, secondColon);
      String amount = str.substring(secondColon + 1);

      // Send request to API
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String endpoint;
        if (String(action).equals("balance")) {
          endpoint = String(baseUrl) + "/balance/eur?userId=" + userId;
        } else {
          endpoint = String(baseUrl) + "/payment/outgoing/eur";
        }
        http.begin(endpoint);
        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"action\":\"" + action + "\",\"userId\":" + userId + ",\"amount\":" + amount + "}";
        int httpResponseCode;
        
        if (action == "balance") {
          httpResponseCode = http.GET();
        } else {
          httpResponseCode = http.POST(jsonPayload);
        }

        String responseToSend = "";

        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("HTTP Response code: " + String(httpResponseCode));
          Serial.println("Response: " + response);

          // Parse JSON response
          DynamicJsonDocument doc(2048); // Increased from 1024 to 2048
          DeserializationError error = deserializeJson(doc, response);

          if (error) {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
            responseToSend = "Error:JSON parsing failed";
          } else {
            String responseToSend;
            if (action == "balance") {
              // Extract balance information
              String assetCode = doc["pageProps"]["account"]["assetCode"].as<String>();
              String balance = doc["pageProps"]["account"]["balance"].as<String>();
              
              // Convert balance from cents to euros
              float balanceFloat = balance.toFloat() / 100.0;
              char balanceStr[10];
              dtostrf(balanceFloat, 4, 2, balanceStr);

              responseToSend = "Balance:" + String(balanceStr) + " " + assetCode;

              // Display on OLED
              display.clearBuffer();
              display.setFont(u8g2_font_ncenB08_tr);
              display.drawStr(0, 10, "Balance:");
              display.drawStr(0, 20, (String(balanceStr) + " " + assetCode).c_str());
              display.sendBuffer();
            } else {
              // Existing code for handling other actions
              String amount = doc["paymentDetails"]["receiveAmount"]["value"].as<String>();
              String currency = doc["paymentDetails"]["receiveAmount"]["assetCode"].as<String>();
              bool failed = doc["paymentDetails"]["failed"].as<bool>();

              String status = failed ? "Failed" : "Success";

              responseToSend = status + ":" + amount + " " + currency;

              // Display on OLED
              display.clearBuffer();
              display.setFont(u8g2_font_ncenB08_tr);
              display.drawStr(0, 10, "Payment Status:");
              display.drawStr(0, 20, status.c_str());
              display.drawStr(0, 30, (amount + " " + currency).c_str());
              display.sendBuffer();
            }

            // Send LoRa response
            Serial.println("Sending LoRa response: " + responseToSend);
            display.clearBuffer();
            display.drawStr(0, 10, "Sending response");
            display.sendBuffer();

            state = radio.transmit(responseToSend);

            if (state == RADIOLIB_ERR_NONE) {
              Serial.println(F("Response sent successfully!"));
              display.drawStr(0, 20, "Response sent");
            } else {
              Serial.println(F("Failed to send response"));
              display.drawStr(0, 20, "Failed to send");
            }
            display.sendBuffer();
          }
        } else {
          Serial.print("Error on sending POST: ");
          Serial.println(httpResponseCode);

          responseToSend = "Error:" + String(httpResponseCode);

          // Display error on OLED
          display.clearBuffer();
          display.drawStr(0, 10, "HTTP Error:");
          display.drawStr(0, 20, String(httpResponseCode).c_str());
          display.sendBuffer();
        }

        // Send LoRa response
        Serial.println("Sending LoRa response: " + responseToSend);
        display.clearBuffer();
        display.drawStr(0, 10, "Sending response");
        display.sendBuffer();

        state = radio.transmit(responseToSend);

        if (state == RADIOLIB_ERR_NONE) {
          Serial.println(F("Response sent successfully!"));
          display.drawStr(0, 20, "Response sent");
        } else {
          Serial.println(F("Failed to send response"));
          display.drawStr(0, 20, "Failed to send");
        }
        display.sendBuffer();

        delay(2000);

        http.end();
      }
    } else {
      Serial.println("Invalid message format");
      display.clearBuffer();
      display.drawStr(0, 10, "Invalid msg format");
      display.sendBuffer();
      delay(2000);
    }

    // Restart receiver
    Serial.println(F("Waiting for next message..."));
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 10, "Waiting for LoRa");
    display.sendBuffer();

    radio.startReceive();
    enableInterrupt = true;
  }
}
