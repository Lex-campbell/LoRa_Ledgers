# LoRa HTTP Gateway

## Overview

LoRa HTTP Gateway is a crucial component of the LoRa Finger banking system, designed to bridge the gap between rural areas with limited internet connectivity and the global financial network. This project acts as an intermediary, receiving LoRa transmissions from LoRa Finger devices and relaying them to the Interledger network via HTTP.

## Features

- LoRa communication for receiving long-range, low-power transmissions
- WiFi connectivity for internet access
- HTTP client for communicating with the Interledger API
- OLED display for status information and debugging
- Support for balance inquiries, sending, and receiving funds

## System Architecture

LoRa HTTP Gateway is part of a larger system:

1. LoRa Finger devices: Send financial transaction requests via LoRa
2. LoRa HTTP Gateway: Receives LoRa transmissions and forwards them to the internet
3. Interledger Network: Processes cross-border payments and financial transactions

This gateway enables rural LoRa Finger devices to access global financial services without direct internet connectivity.

## Hardware Requirements

- Heltec WiFi LoRa 32 V3 board
- LoRa antenna
- OLED Display (integrated in Heltec board)

## Software Dependencies

- Arduino framework
- PlatformIO
- RadioLib for LoRa communication
- WiFi library for internet connectivity
- HTTPClient for API requests
- U8g2 library for OLED display

## Setup and Installation

1. Clone this repository
2. Open the project in PlatformIO
3. Update the WiFi credentials in the code:
   ```cpp
   const char* ssid = "Your_SSID";
   const char* password = "Your_Password";
   ```
4. Update the API endpoint if necessary:
   ```cpp
   const char* apiUrl = "https://your-api-endpoint.com";
   ```
5. Build and upload the code to your Heltec WiFi LoRa 32 V3 board

## Usage

1. Power on the device
2. The gateway will automatically connect to WiFi and start listening for LoRa transmissions
3. When a LoRa message is received, it will be forwarded to the API
4. The API response will be sent back to the LoRa Finger device
5. Status updates are displayed on the OLED screen

## Contributing

Contributions to LoRa HTTP Gateway are welcome! Please feel free to submit pull requests, create issues or spread the word.

## Acknowledgements

This project was developed as part of the 2024 Interledger hackathon, aiming to address financial inclusion challenges in underserved communities.

## Related Projects

- [LoRa Finger]: The companion project that captures fingerprints and initiates financial transactions in rural areas without internet connectivity.
