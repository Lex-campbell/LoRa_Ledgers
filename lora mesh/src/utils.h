#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Send a message to Telegram bot
void SendTelegram(String message);

#endif // UTILS_H
