/*
 * Non-volatile storage management implementation
 */

#include "preferences_storage.h"

Preferences preferences;

const char* PREF_NAMESPACE = "wifiscan";
const char* PREF_KEY_SCAN_SPEED = "scan_speed";  // Slider value (0-100)

void saveScanSpeed(uint8_t slider_value) {
    preferences.begin(PREF_NAMESPACE, false);
    preferences.putUChar(PREF_KEY_SCAN_SPEED, slider_value);
    preferences.end();
}

uint8_t loadScanSpeed(uint8_t default_value) {
    preferences.begin(PREF_NAMESPACE, true);  // Read-only mode
    uint8_t value = preferences.getUChar(PREF_KEY_SCAN_SPEED, default_value);
    preferences.end();
    return value;
}



