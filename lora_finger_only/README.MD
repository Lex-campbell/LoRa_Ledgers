# LoRa Finger

## Overview

LoRa Finger is designed to bring banking services to unbanked populations in rural areas with limited internet connectivity. This project uses LoRa technology, fingerprint authentication, and the Interledger Protocol to enable secure financial transactions without the need for traditional banking infrastructure, internet access, or smartphones.

## Project Links
- [Presentation](https://docs.google.com/presentation/d/1aF7vJZvEuzK8EpxCkDC1QGUGaDHIS9G-RVnuXwsI3x4/edit#slide=id.g2e5d273ee9f_0_23)
- [Video Demonstration](https://youtu.be/0hlodROddWo)

## Team

Name: LoRa Ledgers
- Lex Campbell
- Byron Coetsee

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

## Learnings

- The fingerprint sensor is finnicky and requires calibration.
- The OLED display is small and hard to see in bright sunlight.
- LoRa is great for long range and low power
- Payment authentication flows on Interledger Open Payments

## Achievements

- We learned a lot about the Interledger Protocol and Open Payments
- We learned a lot about the Heltec WiFi LoRa 32 V3 board
- We learned a lot about the fingerprint sensor (it was a nightmare)

## The Future

- being able to roll this out to merchants, who can accept payments from customers, and send funds to their bank accounts using fingerprint authentication instead of credit cards or smartphones. 

- KYC and KYB process to prevent abuse and fraud


## Contributing

Contributions to LoRa Finger are welcome! Please feel free to submit pull requests, create issues or spread the word.


## Acknowledgements

This project was developed as part of the 2024 Interledger hackathon, aiming to address financial inclusion challenges in underserved communities.

## Related Projects

- [LoRa HTTP]: The companion project that serves as a gateway between LoRa Finger devices and the internet, enabling communication with the Interledger network.
