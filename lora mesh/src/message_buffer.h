#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <vector>
#include <Arduino.h>
#include "message.h"

class MessageBuffer {
private:
    struct MessageRecord {
        String messageId;
        String senderId; 
        unsigned long timestamp;
    };

    static const size_t MAX_RECORDS = 100;
    std::vector<MessageRecord> records;

public:
    bool IsSeen(const Message& msg);
    void AddMessage(const Message& msg);
};

extern MessageBuffer messageBuffer;
extern Transaction pendingTransaction;

#endif // MESSAGE_BUFFER_H
