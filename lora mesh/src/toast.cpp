#include <Arduino.h>
#include <U8g2lib.h>

class Toast {
private:
    U8G2_SSD1306_128X64_NONAME_F_SW_I2C& display;
    String message;
    unsigned long startTime;
    unsigned long duration;
    bool isShowing;

public:
    Toast(U8G2_SSD1306_128X64_NONAME_F_SW_I2C& disp) : display(disp), isShowing(false) {}

    void show(const String& msg, unsigned long durationMs) {
        message = msg;
        startTime = millis();
        duration = durationMs;
        isShowing = true;
    }

    void update() {
        if (!isShowing) return;

        if (millis() - startTime < duration) {
            display.clearBuffer();
            display.setFont(u8g2_font_NokiaSmallPlain_te);
            
            int16_t width = display.getStrWidth(message.c_str());
            int16_t height = display.getAscent() - display.getDescent();
            
            int16_t x = (128 - width) / 2;
            int16_t y = 32 + height / 2;
            
            display.drawRBox(x - 5, y - height - 5, width + 10, height + 10, 3);
            display.setDrawColor(0);
            display.drawStr(x, y, message.c_str());
            display.setDrawColor(1);
            
            display.sendBuffer();
        } else {
            isShowing = false;
        }
    }

    bool isActive() const {
        return isShowing;
    }
};
