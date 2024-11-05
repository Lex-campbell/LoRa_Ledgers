#include "Arduino.h"
#include "SPI.h"
#include <U8g2lib.h>
#include <RadioLib.h>
#include "images.h"
#include <Wire.h>
#include "WiFi.h"
#include "message.h"
#include "payments.h"
#include "message_buffer.h"
#include <ArduinoJson.h>
#include "lora.h"
#include "utils.h"
#include "oled.h"

#define VEXT_CTRL       36
#define PRG_BUTTON_PIN  0

unsigned long LAST_HEARTBEAT_TIME = 0;
static const unsigned long HEARTBEAT_INTERVAL = 60000;

void WIFISetUp(void) {
    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    struct Network {
        const char* ssid;
        const char* password;
    };

    Network networks[] = {
        {"🍑", "808808808"},
        {"AckAck", "808808808"},
        {"Stitch Guest", "please$titcharoundforlunch"},
        {"LC iPhone", "spacetime"}
    };
    
    int networkIndex = 0;
    byte totalAttempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && totalAttempts < 20) {
        Network currentNetwork = networks[networkIndex];
        
        WiFi.begin(currentNetwork.ssid, currentNetwork.password);
        delay(100);

        Serial.printf("Trying network: %s\n", currentNetwork.ssid);
        show("Trying: " + String(currentNetwork.ssid));

        // Try this network for 3 seconds
        byte attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 3) {
            delay(1000);
            attempts++;
            totalAttempts++;
            Serial.println("Connecting...");
        }

        // Switch to next network if this one failed
        if (WiFi.status() != WL_CONNECTED) {
            networkIndex = (networkIndex + 1) % (sizeof(networks) / sizeof(networks[0]));
            WiFi.disconnect();
            delay(100);
        }
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected.");
        show("Connected.");
    } else {
        Serial.println("Failed to connect.");
        show("Failed to connect.");
    }
    show("WiFi setup done");
    delay(500);
}

// SENDING

void send(Message msg) {
    String jsonString = Message::encode(msg);
    String compressedString = Message::compress(jsonString);
    console("Sending:\n" + jsonString);// + "\nOriginal size: " + jsonString.length() + " bytes\nCompressed size: " + compressedString.length() + " bytes");
    
    lora.send(compressedString);
    messageBuffer.AddMessage(msg);
}

void sendComplete() {
    lora.startListening();
}

// TX HANDLING

void completePendingTx(Transaction tx) {
    show(tx.humanStringState());
    pendingTransaction = Transaction();
}

void forwardOrProcess(const Message& receivedMessage) {
    Transaction tx = receivedMessage.tx;
    if (WiFi.status() == WL_CONNECTED && tx.id.length() > 0) {
        // Process the transaction
        console("Processing:\n" + tx.humanString());
        show(tx.humanString());
        tx = ProcessTransaction(tx);
        console("Processed:\n" + tx.humanString());

        if (pendingTransaction.id != tx.id) {
            // if the in-scope tx is not the on-device pending tx, send a response
            show("Responding:\n" + tx.humanString());
            Message msg = Message::create("", tx);
            send(msg);
        } else {
            completePendingTx(tx);
        }
    } else {
        // Forward the same (or new) message if not connected to WiFi
        Message msg = receivedMessage;
        if (tx.id.length() > 0) {
            show("Forwarding:\n" + msg.tx.humanString());
        } else {
            show(msg.message);
        }
        send(msg);
    }
}

// RECEIVING

void onReceive(String str) {
    digitalWrite(LED, LOW);  // Turn off LED when not in receiving mode

    String decompressedString = Message::decompress(str);
    Message receivedMessage = Message::decode(decompressedString);
    console("Received:\n" + decompressedString + "\nOriginal size: " + str.length() + " bytes\nDecompressed size: " + decompressedString.length() + " bytes");

    if (messageBuffer.IsSeen(receivedMessage)) {
        console("Message already seen, ignoring");
        blink();
        lora.startListening();
        return;
    }

    if (receivedMessage.tx.id.length() == 0) {
        show(receivedMessage.message);
        lora.startListening();
        return;
    }

    if (pendingTransaction.id == receivedMessage.tx.id) {
        console("Receieved response for pending transaction: " + receivedMessage.tx.humanString());
        completePendingTx(receivedMessage.tx);
        lora.startListening();
        return;
    }

    forwardOrProcess(receivedMessage);
}

// INPUT

// Temporary function to send a random transaction
void sendTx() {
    Transaction tx;
    if (pendingTransaction.id.length() > 0) {
        tx = pendingTransaction;
    } else {
        double amount = random(1, 1000); // Random amount between 1-1000
        tx = Transaction::create(amount, "ZAR", "", "YOLO"); // Empty from/to for now
        pendingTransaction = tx;
    }
    
    forwardOrProcess(Message::create("", tx));
}

void handleButtonPressed() {
    // Check if the PRG button is pressed
    if (digitalRead(PRG_BUTTON_PIN) == LOW) {
        // Debounce the button press
        delay(50);
        if (digitalRead(PRG_BUTTON_PIN) == LOW) {
            sendTx();

            // Wait for the button to be released
            while (digitalRead(PRG_BUTTON_PIN) == LOW) {
                delay(10);
            }
        }
    }
}

// TELEGRAM

String getTelegramMessages() {
    if (WiFi.status() != WL_CONNECTED) {
        return "";
    }

    static unsigned long lastMessageId = 0;
    String response = "";
    String messageId = "";

    HTTPClient http;
    
    String url = "https://api.telegram.org/" + TELEGRAM_BOT_TOKEN + "/getUpdates";
    if (lastMessageId > 0) {
        url += "?offset=" + String(lastMessageId + 1);
    }

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        JsonDocument doc;
        doc.clear();
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            JsonArray results = doc["result"].as<JsonArray>();
            
            for (JsonVariant result : results) {
                lastMessageId = result["update_id"].as<unsigned long>();
                
                if (result["message"]["message_id"] && result["message"]["text"]) {
                    messageId = String(result["message"]["message_id"].as<unsigned long>());
                    response = result["message"]["text"].as<String>();
                    break;
                }
            }
        } else {
            console("Failed to parse JSON response: " + String(error.c_str()));
        }
    }

    http.end();

    if (messageId.length() > 0 && response.length() > 0) {
        return messageId + ":" + response;
    }
    return "";
}

void listenForTelegramMessages() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    static unsigned long lastTelegramCheck = 0;
    const unsigned long TELEGRAM_CHECK_INTERVAL = 1000;
    
    if (millis() - lastTelegramCheck < TELEGRAM_CHECK_INTERVAL) {
        return;
    }
    
    lastTelegramCheck = millis();

    String response = getTelegramMessages();
    
    if (response.length() > 0) {
        int colonPos = response.indexOf(':');
        String messageId = response.substring(0, colonPos);
        String messageText = response.substring(colonPos + 1);
        
        Message msg = Message::create(messageText);
        msg.id = messageId;
        
        forwardOrProcess(msg);
    }
}


// LIFECYCLE

void sendHeartbeat() {
    if (millis() - LAST_HEARTBEAT_TIME >= HEARTBEAT_INTERVAL) { // Send heartbeat every 60 seconds
        LAST_HEARTBEAT_TIME = millis();

        console("Sending heartbeat");
        Message msg = Message::create("heartbeat");
        send(msg);

        // Blink LED 3 times quickly
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED, HIGH);
            delay(50);
            digitalWrite(LED, LOW);
            delay(50);
        }
    }
}

void setup(void) {
    Serial.begin(115200);

    pinMode(VEXT_CTRL, OUTPUT); 
    digitalWrite(VEXT_CTRL, LOW); // Enable Vext power

    // Initialize display
    display.begin();
    display.setFont(u8g2_font_NokiaSmallPlain_te);
    
    pinMode(LED, OUTPUT);
    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);  // Configure the PRG button pin as input with internal pull-up resistor

    uint64_t chipid = ESP.getEfuseMac();  // The chip ID is essentially its MAC address(length: 6 bytes).
    Serial.printf("ESP32ChipID=%04X", (uint16_t)(chipid>>32));  // print High 2 bytes
    Serial.printf("%08X\n", (uint32_t)chipid);  // print Low 4bytes.

    if (chipid == 0xAC0D3B43CA48) {
        WIFISetUp();
    }

    lora.begin();
    lora.setReceiveCallback(onReceive);

    console("[SX1262] Sending first packet ... ");
    show("Sending first packet..");

    Message msg = Message::create("hello");
    send(msg);
}

void loop() {
    handleButtonPressed();
    updateScreen();
    lora.handleLoop();
    listenForTelegramMessages();
}