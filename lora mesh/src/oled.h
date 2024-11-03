#ifndef OLED_H
#define OLED_H

#include <Arduino.h>
#include <U8g2lib.h>

#define OLED_RESET      21 
#define OLED_SDA        17
#define OLED_SCL        18

// Display the logo bitmap
void logo();

// Display text on the OLED screen with optional signal strength indicator
void show(String data, bool showRssi = true);

// Update the screen
void updateScreen();

extern U8G2_SSD1306_128X64_NONAME_F_SW_I2C display;

#endif // OLED_H
