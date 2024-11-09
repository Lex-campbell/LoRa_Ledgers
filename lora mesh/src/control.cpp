#include <Arduino.h>
#include <vector>
#include <string>
#include "utils.h"
#include "control.h"

Command parseCommand(const String& input) {
    Command cmd;
    // Check if it's a command (starts with /)
    if (!input.startsWith("/")) {
        return cmd;
    }

    // Split the input string by spaces
    std::vector<String> parts;
    int start = 1; // Skip the initial /
    int space;
    
    while ((space = input.indexOf(' ', start)) != -1) {
        parts.push_back(input.substring(start, space));
        start = space + 1;
    }
    parts.push_back(input.substring(start)); // Get the last part

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

    // Get this device's chip ID
    uint64_t chipid = ESP.getEfuseMac();
    char this_device[13];
    snprintf(this_device, sizeof(this_device), "%04X%08X", (uint16_t)(chipid>>32), (uint32_t)chipid);

    // Check if this command is for this device
    if (target == "all" || target == String(this_device)) {
        console("Entering standby mode for " + String(duration) + " minutes");
        // Calculate sleep time in microseconds (duration is in minutes)
        uint64_t sleep_time = duration * 60 * 1000000ULL;
        
        // Configure wake up on GPIO0 (PRG button)
        esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 0);
        
        // Put device into deep sleep
        esp_deep_sleep(sleep_time);
    }
}

void processCommand(const String& commandStr) {
    Command cmd = parseCommand(commandStr);
    
    if (cmd.name.isEmpty() || cmd.target.isEmpty()) {
        console("Invalid command format");
        return;
    }

    // Command routing
    if (cmd.name == "standby") {
        executeStandbyCommand(cmd.target, cmd.params);
    } else {
        console("Unknown command: " + cmd.name);
    }
}
