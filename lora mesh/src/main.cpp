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
#include "lora.h"

#define OLED_RESET      21 
#define OLED_SDA        17
#define OLED_SCL        18

U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA, /* reset=*/ OLED_RESET);   // All Boards without Reset of the Display

#define VEXT_CTRL       36
#define PRG_BUTTON_PIN  0

unsigned int counter = 0;
int LED = 35;

unsigned long lastHeartbeatTime = 0;
unsigned long heartbeatInterval = 60000;
unsigned long lastScreenRefreshTime = 0;
unsigned long screenRefreshInterval = 3000;

// Message lastReceivedMessage;
Transaction pendingTransaction;

void logo() {
    display.clearBuffer();
    display.drawXBM(0, 5, logo_width, logo_height, (const unsigned char *)bitty_logo);
    display.sendBuffer();
    delay(1000);
}

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
        {"AckAck", "808808808"}
    };
    
    int networkIndex = 0;
    byte totalAttempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && totalAttempts < 20) {
        Network currentNetwork = networks[networkIndex];
        
        WiFi.begin(currentNetwork.ssid, currentNetwork.password);
        delay(100);

        Serial.printf("Trying network: %s\n", currentNetwork.ssid);
        display.clearBuffer();
        display.drawStr(0, 10, ("Trying: " + String(currentNetwork.ssid)).c_str());
        display.sendBuffer();

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

    display.clearBuffer();
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected.");

        display.clearBuffer();
        display.drawStr(0, 10, "Connected.");
        display.sendBuffer();
    } else {
        Serial.println("Failed to connect.");
        display.clearBuffer();
        display.drawStr(0, 10, "Failed to connect.");
        display.sendBuffer();
    }
    display.drawStr(0, 20, "WiFi setup done");
    display.sendBuffer();
    delay(500);
}

String getTimeString() {
    char timeStr[9];
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
    return String(timeStr);
}

void blink() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);
        delay(50);
    }
}

void console(String data) {
    Serial.println(data.c_str());
}

void show(String data) {
    int y = 8;
    int lineHeight = 8;
    int maxWidth = 128; // Display width in pixels
    
    display.clearBuffer();

    while (data.length() > 0 && y < 64) { // Stop if we reach bottom of display
        String line = "";
        int charWidth = 6; // Approximate width of each character
        int charsPerLine = maxWidth / charWidth;
        
        // Check for newline first
        int newlinePos = data.indexOf('\n');
        if (newlinePos != -1 && newlinePos < charsPerLine) {
            // Print up to newline
            line = data.substring(0, newlinePos);
            data = data.substring(newlinePos + 1);
        } else {
            // Word wrap
            if (data.length() > charsPerLine) {
                // Look for last space within width limit
                int lastSpace = data.substring(0, charsPerLine).lastIndexOf(' ');
                if (lastSpace != -1) {
                    line = data.substring(0, lastSpace);
                    data = data.substring(lastSpace + 1);
                } else {
                    // No space found, just cut at width
                    line = data.substring(0, charsPerLine);
                    data = data.substring(charsPerLine);
                }
            } else {
                // Remaining text fits on one line
                line = data;
                data = "";
            }
        }
        
        display.drawStr(0, y, line.c_str());
        y += lineHeight;
    }
    
    int16_t rssi = lora.getRSSI();
    float signalStrength = (abs(rssi) - 30) / 70.0 * 100;
    signalStrength = 100.0f - signalStrength;  // Invert the scale
    signalStrength = max(0.0f, min(100.0f, signalStrength));
    
    display.drawStr(0, 64, ("Signal: " + String(int(signalStrength)) + "%").c_str());
    display.sendBuffer();
    lastScreenRefreshTime = millis();
}

void startListening() {
    console("Listening...");
    lora.startListening();
    digitalWrite(LED, HIGH);  // Turn on LED when in receiving mode
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
    startListening();
}

// TX HANDLING
void completePendingTx(Transaction tx) {
    show(tx.humanStringState());
    pendingTransaction = Transaction();
}

// Forward or process a transaction
void forwardOrProcess(const Message& receivedMessage) {
    Transaction tx = receivedMessage.tx;
    if (WiFi.status() == WL_CONNECTED) {
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
        if (msg.id.length() == 0) {
            msg = Message::create("", tx);
            show("Broadcasting:\n" + msg.tx.humanString());
        } else {
            show("Forwarding:\n" + msg.tx.humanString());
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
        startListening();
        return;
    }

    if (receivedMessage.tx.id.length() == 0) {
        show(receivedMessage.message);
        startListening();
        return;
    }

    if (pendingTransaction.id == receivedMessage.tx.id) {
        console("Receieved response for pending transaction: " + receivedMessage.tx.humanString());
        completePendingTx(receivedMessage.tx);
        startListening();
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

// LIFECYCLE
void sendHeartbeat() {
    if (millis() - lastHeartbeatTime >= heartbeatInterval) { // Send heartbeat every 60 seconds
        lastHeartbeatTime = millis();

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

void refreshScreen() {
    if (millis() - lastScreenRefreshTime >= screenRefreshInterval) {
        lastScreenRefreshTime = millis();
        if (pendingTransaction.state == Transaction::STATE_PENDING) {
            show("(O. O)\n\n" + pendingTransaction.humanStringState());
        } else {
            show("(-. -)  zzz");
        }
    }
}

void setup(void) {
    Serial.begin(115200);

    pinMode(VEXT_CTRL, OUTPUT); 
    digitalWrite(VEXT_CTRL, LOW); // Enable Vext power

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
    refreshScreen();
    lora.handleLoop();
}