#include "transaction.h"
#include "telegram.h"

Transaction Transaction::create(double amount, const String& currency, const String& from, const String& to) {
    Transaction tx;
    
    // Generate random UUID for transaction ID
    char uuid[11];  // 10 chars + null terminator
    uint32_t r1 = esp_random();
    uint32_t r2 = esp_random();
    snprintf(uuid, sizeof(uuid), "%04X%06X", 
            (uint16_t)(r1 >> 16), (uint32_t)(r2 & 0xFFFFFF));

    // Get chip ID if from is empty
    String fromId = from;
    if (from.length() == 0) {
        uint64_t chipid = ESP.getEfuseMac();
        char sender_id[37];
        snprintf(sender_id, sizeof(sender_id), "%04X%08X", (uint16_t)(chipid>>32), (uint32_t)chipid);
        fromId = String(sender_id);
    }

    tx.id = String(uuid);
    tx.amount = amount;
    tx.currency = currency;
    tx.from = fromId;
    tx.to = to;
    tx.state = STATE_PENDING;
    return tx;
}

String Transaction::encode(const Transaction& tx) {
    JsonDocument doc;
    doc["id"] = tx.id;
    doc["amount"] = tx.amount;
    doc["currency"] = tx.currency;
    doc["from"] = tx.from;
    doc["to"] = tx.to;
    doc["state"] = tx.state;
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

Transaction Transaction::decode(const String& jsonString) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    
    Transaction tx;
    if (error) {
        tx.state = STATE_ERROR;

        SendTelegram("Failed to parse transaction JSON:\n\n" + jsonString + "\n\n" + String(error.c_str()));
    } else {
        tx.id = doc["id"].as<String>();
        tx.amount = doc["amount"].as<double>();
        tx.currency = doc["currency"].as<String>();
        tx.from = doc["from"].as<String>();
        tx.to = doc["to"].as<String>();
        tx.state = doc["state"].as<String>();
    }
    return tx;
}

String Transaction::humanString() const {
    String output = String(amount) + " " + currency;
    if (from.length() > 0) {
        output += " from " + from;
    }
    if (to.length() > 0) {
        output += " to " + to;
    }
    output += " (" + state + ")";
    return output;
}

String Transaction::humanStringState() const {
    if (state == STATE_SUCCESS) {
        return "Sent " + String(amount) + " " + currency;
    } else if (state == STATE_FAILED) {
        return humanString();
    } else {
        return "Sending " + String(amount) + " " + currency;
    }
}
