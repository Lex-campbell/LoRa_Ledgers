#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <Arduino.h>
#include <Preferences.h>

extern Preferences preferences;
extern bool shouldConnectWiFi;
void loadWiFiSetting();
void setWiFiEnabled(bool enabled);

#endif // PREFERENCES_H
