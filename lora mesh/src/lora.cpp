#include "lora.h"

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
        Serial.println("LoRa initialization failed with code: " + String(state));
        return;
    }

    // Configure radio parameters
    radio->setFrequency(868.0);
    radio->setSpreadingFactor(8);
    radio->setCodingRate(7);
    radio->setBandwidth(125.0);
    radio->setOutputPower(22);
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
    Serial.println("Listening...");
    isTransmitting = false;
    int state = radio->startReceive();
    digitalWrite(35, HIGH);
}

void LoRa::send(String data) {
    Serial.println("Starting transmission...");  // Debug
    transmissionState = radio->startTransmit(data);
    if (transmissionState == RADIOLIB_ERR_NONE) {
        isTransmitting = true;
    } else {
        Serial.println("Failed to start transmission");  // Debug
    }
}

void LoRa::setFlag() {
    isIdle = false;
}

void LoRa::handleLoop() {
    if (isIdle) {
        return;
    }
    Serial.println("Handling LoRa loop");
    isIdle = true;

    if (isTransmitting) {
        if (transmissionState != RADIOLIB_ERR_NONE) {
            Serial.println("Transmission error: " + String(transmissionState));
        } else {
            Serial.println("Transmission completed");
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
            Serial.println("Read error: " + String(state));
        }
    }
}

int16_t LoRa::getRSSI() {
    return radio->getRSSI(true);
}

LoRa lora;
