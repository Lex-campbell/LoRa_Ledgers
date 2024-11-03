#include "utils.h"

void SendTelegram(String message) {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    HTTPClient http;
    http.begin("https://api.telegram.org/bot7520876386:AAGi1FeV9XC6wdHZyc6EVpvuRW7DPHZBgjU/sendMessage");
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"chat_id\":\"795879280\",\"text\":\"" + message + "\"}";
    int httpResponseCode = http.POST(jsonBody);

    if (httpResponseCode > 0) {
        Serial.println("Telegram message sent successfully");
    } else {
        Serial.println("Error sending Telegram message");
    }

    http.end();
}

void console(String data) {
    Serial.println(data.c_str());
}
