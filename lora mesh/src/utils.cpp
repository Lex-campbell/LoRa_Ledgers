#include "utils.h"

void SendTelegram(String message) {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    HTTPClient http;
    http.begin("https://api.telegram.org/bot" + TELEGRAM_BOT_TOKEN + "/sendMessage");
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"chat_id\":\"" + TELEGRAM_CHAT_ID + "\",\"text\":\"" + message + "\"}";
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

void blink() {
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED, HIGH);
        delay(50);
        digitalWrite(LED, LOW);
        delay(50);
    }
}

String getTimeString() {
    char timeStr[9];
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);
    return String(timeStr);
}