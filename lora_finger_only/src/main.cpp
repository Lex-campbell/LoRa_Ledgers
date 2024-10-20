#include <Arduino.h>
#include "waveshare_fingerprint.h"
#include <SPI.h>
#include <Wire.h>
#include "RadioLib.h"
#include <U8g2lib.h>
#include <Keypad.h>

// Define the pins for the fingerprint sensor
#define Finger_RST_Pin     45
#define Finger_WAKE_Pin     46
#define Finger_TX_Pin      47
#define Finger_RX_Pin      48

// Create a HardwareSerial object for the fingerprint sensor
HardwareSerial fingerprintSerial(2); // Use UART2

// Create a waveshare_fingerprint object
waveshare_fingerprint fp(&fingerprintSerial, Finger_RST_Pin, Finger_WAKE_Pin);

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

const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {38, 1, 2, 3};     // Keypad Pins 4, 3, 2, 1
byte colPins[COLS] = {4, 5, 6, 7}; // Connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Function declarations
String getAmount(String action);
uint16_t scanFingerprint();
void sendLoRaMessage(uint16_t userId, String amount, String action);

// Add these global variables at the top of your file
volatile bool receivedFlag = false;
volatile bool enableInterrupt = true;

// Add this function to set the flag when a packet is received
void setFlag(void) {
  if(!enableInterrupt) {
    return;
  }
  receivedFlag = true;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Serial.println("Fingerprint scanner test");

  // Initialize OLED display
  Serial.println("Initializing OLED...");
  if (!display.begin()) {
    Serial.println("OLED initialization failed");
    while (1);
  }
  Serial.println("OLED initialized successfully");

  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 10, "LoRa Fingerprint");
  display.drawStr(0, 20, "Scanner Ready");
  display.sendBuffer();
  Serial.println("Initial OLED message displayed");

  // Initialize the fingerprint sensor
  fingerprintSerial.begin(19200, SERIAL_8N1, Finger_RX_Pin, Finger_TX_Pin);
  fp.begin(19200);
  
  // Reset the sensor
  fp.reset();
  delay(2000);

  // Get and print the DSP version
  String dspVersion = fp.get_DSP_version();
  Serial.print("DSP Version: ");
  Serial.println(dspVersion);

  // Get and print the total number of fingerprints
  uint16_t totalFingerprints = fp.total_fingerprints();
  Serial.print("Total fingerprints: ");
  Serial.println(totalFingerprints);

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

  // Set up LoRa parameters (you may need to adjust these based on your requirements)
  radio.setFrequency(915.0); // Set frequency to 915 MHz
  radio.setBandwidth(125.0); // Set bandwidth to 125 kHz
  radio.setSpreadingFactor(7); // Set spreading factor to 7
  radio.setCodingRate(5); // Set coding rate to 4/5
  radio.setOutputPower(10); // Set output power to 10 dBm

  // Set the function that will be called when a packet is received
  radio.setDio1Action(setFlag);
}

void loop() {
  Serial.println("Entering loop");
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 10, "A to send");
  display.drawStr(0, 20, "B to receive");
  display.drawStr(0, 30, "C for balance");
  display.sendBuffer();
  Serial.println("Loop OLED message displayed");

  char key = keypad.getKey();
  
  if (key) {
    Serial.print("Key pressed: ");
    Serial.println(key);
  }
  
  if (key == 'A' || key == 'B' || key == 'C') {
    String action = (key == 'A') ? "send" : (key == 'B') ? "receive" : "balance";
    if (action == "balance") {
      uint16_t identified_user = scanFingerprint();
      if (identified_user != 0) {
        sendLoRaMessage(identified_user, "0", action);
      }
    } else {
      String amount = getAmount(action);
      if (amount != "") {
        uint16_t identified_user = scanFingerprint();
        if (identified_user != 0) {
          sendLoRaMessage(identified_user, amount, action);
        }
      }
    }
  }

  delay(100);  // Small delay to prevent tight looping
}

String getAmount(String action) {
  display.clearBuffer();
  display.drawStr(0, 10, ("How much to " + action + "?").c_str());
  display.drawStr(0, 20, "Press # when done");
  display.sendBuffer();

  String amount = "";
  while (true) {
    char key = keypad.getKey();
    if (key >= '0' && key <= '9') {
      amount += key;
      display.drawStr(0, 30, amount.c_str());
      display.sendBuffer();
    } else if (key == '#') {
      return amount;
    } else if (key == '*') {
      return ""; // Cancel operation
    }
  }
}

uint16_t scanFingerprint() {
  display.clearBuffer();
  display.drawStr(0, 10, "Place finger");
  display.sendBuffer();

  Serial.println("Place your finger on the sensor...");
  uint16_t identified_user = 0;
  auto scan_result = fp.scan_1_to_N(&identified_user);
  
  if (scan_result == waveshare_fingerprint::ACK_SUCCESS) {
    Serial.print("User identified! User ID: ");
    Serial.println(identified_user);
    return identified_user;
  } else if (scan_result == waveshare_fingerprint::ACK_NOUSER) {
    Serial.println("Fingerprint not recognized");
    display.drawStr(0, 20, "Not recognized");
    display.sendBuffer();
  } else {
    Serial.print("Scan failed with error code: ");
    Serial.println(scan_result);
    display.drawStr(0, 20, "Scan failed");
    display.sendBuffer();
  }
  
  delay(2000);
  return 0;
}

void sendLoRaMessage(uint16_t userId, String amount, String action) {
  String message = action + ":" + String(userId) + ":" + amount;
  Serial.print(F("Sending message: "));
  Serial.println(message);
  
  display.clearBuffer();
  display.drawStr(0, 10, "Sending LoRa");
  display.drawStr(0, 20, message.c_str());
  display.sendBuffer();
  
  radio.setOutputPower(17); // Increase output power to 17 dBm (adjust as needed)
  int state = radio.transmit(message);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println(F("Transmission successful!"));
    display.drawStr(0, 30, "Sent successfully");
    display.sendBuffer();
    delay(1000); // Show "Sent successfully" for 1 second
    
    // Wait for response
    display.clearBuffer();
    display.drawStr(0, 10, "Waiting for response");
    display.sendBuffer();
    
    // Start listening for the response
    radio.startReceive();
    enableInterrupt = true;
    receivedFlag = false;
    
    unsigned long startTime = millis();
    bool responseReceived = false;
    String responseStr;

    while (millis() - startTime < 10000) { // Wait for up to 10 seconds
      if(receivedFlag) {
        enableInterrupt = false;
        receivedFlag = false;

        int state = radio.readData(responseStr);

        if (state == RADIOLIB_ERR_NONE) {
          Serial.println(F("Response received!"));
          Serial.println(responseStr);
          responseReceived = true;
          break;
        } else {
          Serial.println(F("Failed to read response"));
        }

        enableInterrupt = true;
      }
      delay(10); // Small delay to prevent tight looping
    }
    
    if (responseReceived) {
      display.clearBuffer();
      display.drawStr(0, 10, "Response received:");
      display.drawStr(0, 20, responseStr.c_str());
      display.sendBuffer();
      delay(3000); // Show response for 3 seconds
    } else {
      Serial.println(F("No response received"));
      display.clearBuffer();
      display.drawStr(0, 10, "No response received");
      display.sendBuffer();
      delay(3000); // Show "No response received" for 3 seconds
    }
  } else {
    Serial.print(F("Transmission failed, code "));
    Serial.println(state);
    display.drawStr(0, 30, "Send failed");
    display.sendBuffer();
    delay(3000); // Show error for 3 seconds
  }

  // Clear display before returning to main menu
  display.clearBuffer();
  display.sendBuffer();
}
