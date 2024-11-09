#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "message.h"
#include "utils.h"
#include "lora.h"
#include "telegram.h"
#include "oled.h"

static unsigned long lastMessageId = 0;

TelegramMessage::TelegramMessage() : messageId(0), text(""), firstName(""), lastName(""), username(""), chatId("") {}

TelegramMessage::TelegramMessage(unsigned long id, String msg, String first, String last, String user, String chat)
    : messageId(id), text(msg), firstName(first), lastName(last), username(user), chatId(chat) {}

bool TelegramMessage::isEmpty() {
    return messageId == 0 && text.isEmpty();
}

String TelegramMessage::toString() {
    if (isEmpty()) return "";
    String str = "";
    if (username.length() > 0) str += "@" + username + "\n";
    str += text;
    return str;
}

TelegramMessage getTelegramMessages() {
    if (WiFi.status() != WL_CONNECTED) {
        return TelegramMessage();
    }

    HTTPClient http;
    
    String url = "https://api.telegram.org/bot" + TELEGRAM_BOT_TOKEN + "/getUpdates";
    if (lastMessageId > 0) {
        url += "?offset=" + String(lastMessageId + 1);
    }

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        JsonDocument doc;
        doc.clear();
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            JsonArray results = doc["result"].as<JsonArray>();
            
            for (JsonVariant result : results) {
                lastMessageId = result["update_id"].as<unsigned long>();
                
                if (result["message"]["message_id"] && result["message"]["text"]) {
                    unsigned long messageId = result["message"]["message_id"].as<unsigned long>();
                    String text = result["message"]["text"].as<String>();
                    String firstName = result["message"]["from"]["first_name"] | "";
                    String lastName = result["message"]["from"]["last_name"] | "";
                    String username = result["message"]["from"]["username"] | "";
                    String chatId = String(result["message"]["chat"]["id"].as<long long>());
                    
                    http.end();
                    console("Telegram message received:\n" 
                           "Message ID: " + String(messageId) + "\n"
                           "Text: " + text + "\n"
                           "From: " + firstName + " " + lastName + "\n" 
                           "Username: @" + username + "\n"
                           "Chat ID: " + chatId);
                    return TelegramMessage(messageId, text, firstName, lastName, username, chatId);
                }
            }
        } else {
            console("Failed to parse JSON response: " + String(error.c_str()));
        }
    }

    http.end();
    return TelegramMessage();
}

void listenForTelegramMessages() {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    static unsigned long lastTelegramCheck = 0;
    const unsigned long TELEGRAM_CHECK_INTERVAL = 5000;
    
    if (millis() - lastTelegramCheck < TELEGRAM_CHECK_INTERVAL) {
        return;
    }
    
    lastTelegramCheck = millis();

    TelegramMessage telegramMsg = getTelegramMessages();
    
    if (!telegramMsg.isEmpty()) {
        Message msg = Message::create(telegramMsg.toString());
        msg.id = String(telegramMsg.messageId);
        
        lora.send(msg);
        show(telegramMsg.toString());
    }
}

void SendTelegram(String message) {
    if (WiFi.status() != WL_CONNECTED) {
        return;
    }

    HTTPClient http;
    http.begin("https://api.telegram.org/bot" + TELEGRAM_BOT_TOKEN + "/sendMessage");
    http.addHeader("Content-Type", "application/json");

    String jsonBody = "{\"chat_id\":\"" + TELEGRAM_CHAT_ID + "\",\"text\":\"" + message + "\"}";
    int httpResponseCode = http.POST(jsonBody);

    if (httpResponseCode == 200) {
        Serial.println("Telegram message sent successfully");
    } else {
        Serial.println("Error sending Telegram message: " + http.getString());
    }

    http.end();
}