#include "message_buffer.h"

bool MessageBuffer::IsSeen(const Message& msg) {
    // Remove old records (older than 1 hour)
    unsigned long currentTime = millis();
    auto it = records.begin();
    while (it != records.end()) {
        if (currentTime - it->timestamp > 3600000) { // 1 hour in milliseconds
            it = records.erase(it);
        } else {
            ++it;
        }
    }

    // Check if message ID already exists
    for (const auto& record : records) {
        if (record.messageId == msg.id) {
            return true;
        }
    }

    // Add new record
    if (records.size() >= MAX_RECORDS) {
        records.erase(records.begin()); // Remove oldest record
    }

    records.push_back({
        msg.id,
        msg.sender,
        currentTime
    });

    return false;
}

void MessageBuffer::AddMessage(const Message& msg) {
    unsigned long currentTime = millis();
    
    // Remove old records if at capacity
    if (records.size() >= MAX_RECORDS) {
        records.erase(records.begin());
    }

    // Add new record
    records.push_back({
        msg.id,
        msg.sender, 
        currentTime
    });
}


// Global instance
MessageBuffer messageBuffer;
Transaction pendingTransaction;