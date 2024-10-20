# LoRa Ledgers: Banking the Unbanked

## Overview

LoRa Ledgers is an innovative solution designed to bring banking services to unbanked populations in rural areas with limited internet connectivity. This project uses LoRa technology, fingerprint authentication, and the Interledger Protocol to enable secure financial transactions without the need for traditional banking infrastructure, internet access, or smartphones.

The project consists of four main components:
1. LoRa Finger: A device for user interaction and transaction initiation
2. LoRa HTTP Gateway: A bridge between LoRa networks and the internet
3. Server: An intermediary that wraps the Interledger API
4. Telegram Bot: An interface for remote management and monitoring

## System Architecture

1. LoRa Finger devices: Deployed in rural areas, these devices capture user fingerprints and transaction details.
2. LoRa HTTP Gateway: Receives LoRa transmissions from LoRa Finger devices and relays them to the server.
3. Server: Processes requests from the LoRa HTTP Gateway and communicates with the Interledger API.
4. Interledger Network: Processes cross-border payments and financial transactions.
5. Telegram Bot: Provides remote management and monitoring capabilities.

This architecture enables rural users to access global financial services without direct internet connectivity, while allowing administrators to manage the system remotely.

## Features

- Fingerprint-based user authentication in place of traditional credit cards or banking apps
- LoRa communication for long-range, low-power data transmission
- Integration with Interledger Protocol for cross-border payments
- OLED display for user interaction and status information
- Keypad input for transaction amounts and menu navigation
- Support for balance inquiries, sending, and receiving funds
- WiFi connectivity in the gateway for internet access
- HTTP client for communicating with the server
- Server-side API wrapping for Interledger integration
- Telegram bot for remote management and monitoring

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

- Arduino framework (for LoRa devices)
- PlatformIO (for LoRa devices)
- Node.js (for server and Telegram bot)
- Express.js (for server)
- node-telegram-bot-api (for Telegram bot)
- Various libraries for LoRa devices (U8g2, RadioLib, ArduinoJson, etc.)

## Setup and Installation

1. Clone the LoRa Ledgers repository
2. Set up the LoRa Finger and LoRa HTTP Gateway:
   - Open each project in PlatformIO
   - Connect hardware components according to pin definitions
   - Update WiFi credentials and API endpoints
   - Build and upload code to Heltec boards
3. Set up the server and Telegram bot:
   - Open the project in Visual Studio Code
   - Configure environment variables in the `.env` file (API keys, endpoints, etc.)
   - Use the provided `launch.json` file to run the server and Telegram bot

## Usage

### LoRa Finger
1. Power on the device
2. Use the keypad to select an action (send, receive, check balance)
3. Follow on-screen prompts and use fingerprint sensor when prompted
4. Wait for the LoRa transmission and response

### LoRa HTTP Gateway
1. Power on the device
2. The gateway will connect to WiFi and listen for LoRa transmissions
3. Received messages are forwarded to the server
4. Responses are sent back to LoRa Finger devices

### Server
- Handles requests from LoRa HTTP Gateway
- Communicates with Interledger API
- Manages user accounts and transactions

### Telegram Bot
- Use Telegram app to interact with the bot
- Monitor system status
- Manage user accounts
- View transaction history
- Perform administrative tasks

## Contributing

Contributions to LoRa Ledgers are welcome! Please feel free to submit pull requests, create issues, or spread the word about any component of the project.

## Acknowledgements

This project was developed as part of the 2024 Interledger hackathon, aiming to address financial inclusion challenges in underserved communities.

## Project Structure

- `/lora_finger_only`: LoRa Finger device code
- `/lora_2_http`: LoRa HTTP Gateway code
- `/server`: Server code wrapping Interledger API
- `/telegram_bot`: Telegram bot for remote management
