#include <Arduino.h>
#include <vector>
#include <string>
#include "utils.h"
#include "control.h"
#include "transaction.h"
#include "message.h"
#include "lora.h"

Command parseCommand(const String& input) {
    Command cmd;
    
    // Handle username prefix if present
    String commandStr = input;
    if (input.startsWith("@")) {
        int newlineAfterUsername = input.indexOf("\n");
        if (newlineAfterUsername != -1) {
            commandStr = input.substring(newlineAfterUsername + 1);
        } else {
            int spaceAfterUsername = input.indexOf(" ");
            if (spaceAfterUsername != -1) {
                commandStr = input.substring(spaceAfterUsername + 1);
            }
        }
    }

    // Check if it's a command (starts with /)
    if (!commandStr.startsWith("/")) {
        return cmd;
    }

    // Split the input string by spaces
    std::vector<String> parts;
    int start = 1; // Skip the initial /
    int space;
    
    while ((space = commandStr.indexOf(' ', start)) != -1) {
        parts.push_back(commandStr.substring(start, space));
        start = space + 1;
    }
    parts.push_back(commandStr.substring(start)); // Get the last part

    if (parts.size() < 2) {
        return cmd;
    }

    cmd.name = parts[0];
    cmd.target = parts[1];
    
    // Add remaining parts as parameters
    for (size_t i = 2; i < parts.size(); i++) {
        cmd.params.push_back(parts[i]);
    }

    return cmd;
}

void executeStandbyCommand(const String& target, const std::vector<String>& params) {
    if (params.empty()) {
        console("Standby command requires duration parameter");
        return;
    }

    int duration = params[0].toInt();
    if (duration <= 0) {
        console("Invalid standby duration");
        return;
    }

    // Check if this command is for this device
    console("Entering standby mode for " + String(duration) + " minutes");
    // Calculate sleep time in microseconds (duration is in minutes)
    uint64_t sleep_time = duration * 60 * 1000000ULL;
    
    // Configure wake up on GPIO0 (PRG button)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
    
        // Put device into deep sleep
    esp_deep_sleep(sleep_time);
}

void executeSendTxCommand(const String& target, const std::vector<String>& params) {
    if (params.empty()) {
        console("Send tx command requires amount parameter");
        return;
    }

    console("Sending transaction from telegram");
    double amount = random(1, 1000); // Random amount between 1-1000
    Message msg = Message::create("", Transaction::create(amount, "ZAR", "", "TELEGRAM"));
    lora.send(msg);
}


void processCommand(const String& commandStr) {
    Command cmd = parseCommand(commandStr);
    
    if (cmd.name.isEmpty() || cmd.target.isEmpty()) {
        console("Invalid command format");
        return;
    }

    if (cmd.target != "all" && cmd.target != getChipID()) {
        console("Command not for this device");
        return;
    }

    // Command routing
    if (cmd.name == "standby") {
        executeStandbyCommand(cmd.target, cmd.params);
    } else if (cmd.name == "sendtx") {
        executeSendTxCommand(cmd.target, cmd.params);
    } else {
        console("Unknown command: " + cmd.name);
    }
}
