#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define LED 35

#define TELEGRAM_BOT_TOKEN String("bot7520876386:AAGi1FeV9XC6wdHZyc6EVpvuRW7DPHZBgjU")
#define TELEGRAM_CHAT_ID String("795879280")

// Send a message to Telegram bot
void SendTelegram(String message);

// Print to console
void console(String data);

// Blink the LED
void blink();

// Get the current time as a string
String getTimeString();

#endif // UTILS_H
