# LoRa Finger

## Overview

LoRa Finger is designed to bring banking services to unbanked populations in rural areas with limited internet connectivity. This project uses LoRa technology, fingerprint authentication, and the Interledger Protocol to enable secure financial transactions without the need for traditional banking infrastructure, internet access, or smartphones.

## Features

- Fingerprint-based user authentication in place of tradtional credit card or banking app
- LoRa communication for long-range, low-power data transmission
- Integration with Interledger Protocol for cross-border payments via LoRa HTTP gateway
- OLED display for user interaction
- Keypad input for transaction amounts and menu navigation
- Support for balance inquiries, sending, and receiving funds

## System Architecture

LoRa Finger works in conjunction with the LoRa HTTP project, which acts as a gateway to the internet:

1. LoRa Finger devices: Deployed in rural areas, these devices capture user fingerprints and transaction details.
2. LoRa HTTP Gateway: Receives LoRa transmissions from LoRa Finger devices and relays them to the Interledger network via HTTP.
3. Interledger Network: Processes cross-border payments and financial transactions.

## Hardware Requirements

- Heltec WiFi LoRa 32 V3 board
- Waveshare Fingerprint Sensor
- OLED Display
- 4x4 Keypad
- LoRa antenna

## Software Dependencies

- Arduino framework
- PlatformIO
- U8g2 library for OLED display
- RadioLib for LoRa communication
- ArduinoJson for data serialization
- Keypad library for keypad input

## Setup and Installation

1. Clone this repository
2. Open the project in PlatformIO
3. Connect the hardware components according to the pin definitions in the code
4. Build and upload the code to your Heltec WiFi LoRa 32 V3 board

## Usage

1. Power on the device
2. Use the keypad to select an action:
   - 'A' to send funds
   - 'B' to receive funds
   - 'C' to check balance
3. Follow the on-screen prompts to enter amounts (if applicable)
4. Place your finger on the fingerprint sensor when prompted
5. Wait for the LoRa transmission and response

## Contributing

Contributions to LoRa Finger are welcome! Please feel free to submit pull requests, create issues or spread the word.


## Acknowledgements

This project was developed as part of the 2024 Interledger hackathon, aiming to address financial inclusion challenges in underserved communities.

## Related Projects

- [LoRa HTTP]: The companion project that serves as a gateway between LoRa Finger devices and the internet, enabling communication with the Interledger network.
