#include "lora.h"
#include "utils.h"
#include "message.h"
#include "message_buffer.h"

void onInterruptStatic() {
    lora.setFlag();
}

LoRa::LoRa() {
    radio = new SX1262(new Module(/*cs*/LORA_NSS_PIN, /*irq*/LORA_DIO1_PIN, /*rst*/LORA_RESET_PIN, /*gpio*/LORA_BUSY_PIN));
    isIdle = true;
    isTransmitting = false;
    transmissionState = RADIOLIB_ERR_NONE;
}

void LoRa::begin() {
    SPI.begin(LORA_CLK, LORA_MISO, LORA_MOSI, LORA_NSS_PIN);

    int state = radio->begin();
    if (state != RADIOLIB_ERR_NONE) {
        console("LoRa initialization failed with code: " + String(state));
        return;
    }

    // Range: 137 MHz to 1020 MHz depending on module
    // Higher freq = shorter range but better penetration
    radio->setFrequency(868.0);

    // Range: 6-12, higher = longer range but slower data rate
    // SF8 gives good balance of range vs speed
    radio->setSpreadingFactor(8);

    // Range: 5-8, higher = more error correction but lower data rate
    // 4/7 provides good error correction while maintaining decent speed
    radio->setCodingRate(7);

    // Common values: 7.8, 10.4, 15.6, 20.8, 31.25, 41.7, 62.5, 125, 250, 500 kHz
    // Wider = faster data rate but shorter range and less noise immunity
    radio->setBandwidth(125.0);

    // Range: -17 to 22 dBm
    // Higher power = longer range but more power consumption
    // 22 dBm is max allowed in most regions
    radio->setOutputPower(22);

    // Used to identify network - only nodes with same sync word communicate
    // Can be any value 0x00-0xFF
    radio->setSyncWord(0x45);

    // Set up the interrupt using direct pin attachment
    radio->setDio1Action(onInterruptStatic);

}

void LoRa::standby() {
    radio->standby();
}

void LoRa::setReceiveCallback(void (*callback)(String)) {
    onReceiveCallback = callback;
}

void LoRa::startListening() {
    console("Listening...");
    isTransmitting = false;
    int state = radio->startReceive();
    digitalWrite(LED, HIGH);
}

void LoRa::send(Message msg) {
    String jsonString = Message::encode(msg);
    String compressedString = Message::compress(jsonString);
    console("Sending:\n" + jsonString);
    
    transmissionState = radio->startTransmit(compressedString);
    if (transmissionState == RADIOLIB_ERR_NONE) {
        isTransmitting = true;
        messageBuffer.AddMessage(msg);
    } else {
        console("Failed to start transmission");
    }
}

void LoRa::setFlag() {
    isIdle = false;
}

void LoRa::handleLoop() {
    if (isIdle) {
        return;
    }
    console("Handling LoRa loop");
    isIdle = true;

    if (isTransmitting) {
        if (transmissionState != RADIOLIB_ERR_NONE) {
            console("Transmission error: " + String(transmissionState));
        } else {
            console("Transmission completed");
        }
        startListening();
    } else {
        String str;
        int state = radio->readData(str);
        if (state == RADIOLIB_ERR_NONE) {
            radio->standby();
            if (onReceiveCallback && str.length() > 0) {
                onReceiveCallback(str);
            }
        } else {
            console("Read error: " + String(state));
        }
    }
}

int16_t LoRa::getRSSI() {
    return radio->getRSSI(true);
}

LoRa lora;
