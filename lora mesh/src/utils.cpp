#include "utils.h"

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