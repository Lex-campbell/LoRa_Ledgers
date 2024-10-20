# LoRa Ledgers: Banking the Unbanked

## Overview

LoRa Ledgers is an innovative solution designed to bring banking services to unbanked populations in rural areas with limited internet connectivity. This project uses LoRa technology, fingerprint authentication, and the Interledger Protocol to enable secure financial transactions without the need for traditional banking infrastructure, internet access, or smartphones.

The project consists of two main components:
1. LoRa Finger: A device for user interaction and transaction initiation
2. LoRa HTTP Gateway: A bridge between LoRa networks and the internet

## System Architecture

1. LoRa Finger devices: Deployed in rural areas, these devices capture user fingerprints and transaction details.
2. LoRa HTTP Gateway: Receives LoRa transmissions from LoRa Finger devices and relays them to the Interledger network via HTTP.
3. Interledger Network: Processes cross-border payments and financial transactions.

This architecture enables rural users to access global financial services without direct internet connectivity.

## Features

- Fingerprint-based user authentication in place of traditional credit cards or banking apps
- LoRa communication for long-range, low-power data transmission
- Integration with Interledger Protocol for cross-border payments
- OLED display for user interaction and status information
- Keypad input for transaction amounts and menu navigation
- Support for balance inquiries, sending, and receiving funds
- WiFi connectivity in the gateway for internet access
- HTTP client for communicating with the Interledger API

## Hardware Requirements

### LoRa Finger
- Heltec WiFi LoRa 32 V3 board
- Waveshare Fingerprint Sensor
- OLED Display
- 4x4 Keypad
- LoRa antenna

### LoRa HTTP Gateway
- Heltec WiFi LoRa 32 V3 board
- LoRa antenna
- OLED Display (integrated in Heltec board)

## Software Dependencies

- Arduino framework
- PlatformIO
- U8g2 library for OLED display
- RadioLib for LoRa communication
- ArduinoJson for data serialization
- Keypad library for keypad input
- WiFi library for internet connectivity
- HTTPClient for API requests

## Setup and Installation

1. Clone both the LoRa Finger and LoRa HTTP Gateway repositories
2. Open each project in PlatformIO
3. For LoRa Finger:
   - Connect the hardware components according to the pin definitions in the code
4. For LoRa HTTP Gateway:
   - Update the WiFi credentials and API endpoint in the code
5. Build and upload the code to your Heltec WiFi LoRa 32 V3 boards

## Usage

### LoRa Finger
1. Power on the device
2. Use the keypad to select an action:
   - 'A' to send funds
   - 'B' to receive funds
   - 'C' to check balance
3. Follow the on-screen prompts to enter amounts (if applicable)
4. Place your finger on the fingerprint sensor when prompted
5. Wait for the LoRa transmission and response

### LoRa HTTP Gateway
1. Power on the device
2. The gateway will automatically connect to WiFi and start listening for LoRa transmissions
3. When a LoRa message is received, it will be forwarded to the API
4. The API response will be sent back to the LoRa Finger device
5. Status updates are displayed on the OLED screen

## Contributing

Contributions to LoRa Ledgers are welcome! Please feel free to submit pull requests, create issues or spread the word for either the LoRa Finger or LoRa HTTP Gateway components.

## Acknowledgements

This project was developed as part of the 2024 Interledger hackathon, aiming to address financial inclusion challenges in underserved communities.

## Project Repositories

- [LoRa Finger](https://github.com/Lex-campbell/LoRa_Ledgers/tree/main/lora_finger_only): The device for capturing fingerprints and initiating financial transactions in rural areas.
- [LoRa HTTP Gateway](https://github.com/Lex-campbell/LoRa_Ledgers/tree/main/lora_2_http): The gateway that bridges LoRa communications with the internet and Interledger network.
