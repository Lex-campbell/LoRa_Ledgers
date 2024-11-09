#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>
#include <vector>

struct Command {
    String name;
    String target; 
    std::vector<String> params;
};

Command parseCommand(const String& input);
void executeStandbyCommand(const String& target, const std::vector<String>& params);
void processCommand(const String& commandStr);

#endif
