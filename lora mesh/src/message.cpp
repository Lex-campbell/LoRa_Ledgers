#include "message.h"
#include "unishox/unishox2.h"

// const String ACTION_NONE = "None";
// const String ACTION_BALANCE = "Balance";
// const String ACTION_START_TX = "StartTx";
// const String ACTION_PAY = "Pay";
// const String ACTION_PAY_SUCCESS = "PaySuccess";
// const String ACTION_PAY_FAIL = "PayFail";
// const String ACTION_CANCEL = "Cancel";
// const String ACTION_SEND = "Send";

Message Message::create(const String& message, const Transaction& tx) {
    Message msg;
    uint64_t chipid = ESP.getEfuseMac();
    char sender_id[37];
    snprintf(sender_id, sizeof(sender_id), "%04X%08X", (uint16_t)(chipid>>32), (uint32_t)chipid);
    
    // Generate random UUID for message ID
    char uuid[11];  // 10 chars + null terminator
    uint32_t r1 = esp_random();
    uint32_t r2 = esp_random();
    snprintf(uuid, sizeof(uuid), "%04X%06X", 
            (uint16_t)(r1 >> 16), (uint32_t)(r2 & 0xFFFFFF));
             
    msg.id = String(uuid);
    msg.sender = String(sender_id);
    msg.message = message;
    // msg.action = action;
    msg.tx = tx;
    return msg;
}

String Message::encode(const Message& msg) {
    JsonDocument doc;
    doc["id"] = msg.id;
    doc["sender"] = msg.sender;
    doc["message"] = msg.message;
    // doc["action"] = msg.action;
    doc["tx"] = Transaction::encode(msg.tx);
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

Message Message::decode(const String& jsonString) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);
    
    Message msg;
    if (error) {
        msg.sender = "Error";
        msg.message = "Failed to parse JSON";
    } else {
        msg.id = doc["id"].as<String>();
        msg.sender = doc["sender"].as<String>();
        msg.message = doc["message"].as<String>();
        // msg.action = doc["action"].as<String>();
        msg.tx = Transaction::decode(doc["tx"].as<String>());
    }
    return msg;
}

String Message::compress(const String& input) {
    // return input;
    char output[input.length()]; // Output buffer
    int compressed_len = unishox2_compress_simple(input.c_str(), input.length(), output);
    if (compressed_len < 0) {
        return input; // Return original if compression fails
    }
    return String(output, compressed_len);
}

String Message::decompress(const String& input) {
    // return input;
    char output[input.length() * 2]; // Output buffer with extra space for decompression
    int decompressed_len = unishox2_decompress_simple(input.c_str(), input.length(), output);
    if (decompressed_len < 0) {
        return input; // Return original if decompression fails
    }
    return String(output, decompressed_len);
}
