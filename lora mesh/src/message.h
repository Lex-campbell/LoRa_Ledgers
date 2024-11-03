#ifndef MESSAGE_H
#define MESSAGE_H

#include <ArduinoJson.h>
#include "transaction.h"

// extern const String ACTION_NONE;
// extern const String ACTION_BALANCE;
// extern const String ACTION_PAY;
// extern const String ACTION_PAY_SUCCESS;
// extern const String ACTION_PAY_FAIL;
// extern const String ACTION_CANCEL;
// extern const String ACTION_SEND;

struct Message {
    String id;
    String sender;
    String message;
    Transaction tx;

    static Message create(const String& message, const Transaction& tx = Transaction());
    static String encode(const Message& msg);
    static Message decode(const String& jsonString);
    static String compress(const String& input);
    static String decompress(const String& input);
};

#endif // MESSAGE_H