#ifndef LORA_H
#define LORA_H

#include "Arduino.h"
#include <RadioLib.h>
#include "message.h"
// SX1262 has the following connections:
#define LORA_NSS_PIN    8
#define LORA_RESET_PIN  12
#define LORA_DIO1_PIN   14
#define LORA_BUSY_PIN   13

#define LORA_CLK        9
#define LORA_MISO       11
#define LORA_MOSI       10

class LoRa {
private:
    SX1262* radio;
    int transmissionState;
    bool isIdle;
    bool isTransmitting;
    void (*onReceiveCallback)(String);

public:
    LoRa();
    void begin();
    void standby();
    void setReceiveCallback(void (*callback)(String));
    void startListening();
    void send(Message msg);
    void setFlag();
    void handleLoop();
    int16_t getRSSI();
};

extern LoRa lora;

#endif // LORA_H
