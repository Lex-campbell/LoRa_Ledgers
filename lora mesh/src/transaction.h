#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <Arduino.h>
#include <ArduinoJson.h>

class Transaction {
public:
    double amount;
    String currency;
    String id;
    String from;
    String to;
    String state;

    static constexpr const char* STATE_PENDING = "pending";
    static constexpr const char* STATE_SUCCESS = "success"; 
    static constexpr const char* STATE_FAILED = "failed";
    static constexpr const char* STATE_CANCELLED = "cancelled";
    static constexpr const char* STATE_EXPIRED = "expired";
    static constexpr const char* STATE_ERROR = "error";

    static Transaction create(double amount, const String& currency, const String& from, const String& to);
    static String encode(const Transaction& tx);
    static Transaction decode(const String& jsonString);
    String humanString() const;
    String humanStringState() const;
};

#endif // TRANSACTION_H
