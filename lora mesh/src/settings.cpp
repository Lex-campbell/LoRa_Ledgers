#include "settings.h"
#include "oled.h"

// Global declarations
Preferences preferences;
bool shouldConnectWiFi = true;  // Default value

void loadWiFiSetting() {
    preferences.begin("lora-mesh", true);
    shouldConnectWiFi = preferences.getBool("wifi-enabled", true);  // true is the default if key doesn't exist
    preferences.end();
}

void setWiFiEnabled(bool enabled) {
    preferences.begin("lora-mesh", false);  // Open preferences in RW mode
    shouldConnectWiFi = enabled; 
    preferences.putBool("wifi-enabled", shouldConnectWiFi);
    preferences.end();
    
    show(String("WiFi ") + (enabled ? "enabled" : "disabled"));
}
