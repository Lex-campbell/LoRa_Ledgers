#include "oled.h"
#include "images.h"
#include "lora.h"
#include "message_buffer.h"

// Global display object
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R0, /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA, /* reset=*/ OLED_RESET);   // All Boards without Reset of the Display

static const unsigned long REFRESH_INTERVAL = 3000;
static unsigned long LAST_REFRESH_TIME = 0;

void logo() {
    display.clearBuffer();
    display.drawXBM(0, 5, logo_width, logo_height, (const unsigned char *)bitty_logo);
    display.sendBuffer();
    delay(1000);
}

void show(String data, bool showRssi) {
    LAST_REFRESH_TIME = millis();

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
    
    if (showRssi) {
        display.drawStr(0, 64, ("Signal: " + String(int(signalStrength)) + "%").c_str());
    }

    display.sendBuffer();
}

void updateScreen() {
    if (millis() - LAST_REFRESH_TIME >= REFRESH_INTERVAL) {
        LAST_REFRESH_TIME = millis();
        if (pendingTransaction.state == Transaction::STATE_PENDING) {
            show("(O. O)\n\n" + pendingTransaction.humanStringState());
        } else {
            show("(-. -)  zzz", false);
        }
    }
}