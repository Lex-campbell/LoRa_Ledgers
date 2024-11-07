#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <Arduino.h>

#define TELEGRAM_BOT_TOKEN String("7520876386:AAGi1FeV9XC6wdHZyc6EVpvuRW7DPHZBgjU")
// #define TELEGRAM_CHAT_ID String("795879280")
#define TELEGRAM_CHAT_ID String("-4573880170")

struct TelegramMessage {
    unsigned long messageId;
    String text;
    String firstName;
    String lastName;
    String username;
    String chatId;
    
    TelegramMessage();
    TelegramMessage(unsigned long id, String msg, String first, String last, String user, String chat);
    bool isEmpty();
    String toString();
};

TelegramMessage getTelegramMessages();
void listenForTelegramMessages();
void SendTelegram(String message);

#endif
