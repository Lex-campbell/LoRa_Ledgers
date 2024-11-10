#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LED 35

// Print to console
void console(String data);

// Blink the LED
void blink();

// Get the current time as a string
String getTimeString();

// Get the chip ID as a string
String getChipID();

#endif // UTILS_H
