#include "lora.h"
#include "utils.h"

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
    console("Listening...");
    isTransmitting = false;
    int state = radio->startReceive();
    digitalWrite(LED, HIGH);
}

void LoRa::send(String data) {
    console("Starting transmission...");
    transmissionState = radio->startTransmit(data);
    if (transmissionState == RADIOLIB_ERR_NONE) {
        isTransmitting = true;
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
