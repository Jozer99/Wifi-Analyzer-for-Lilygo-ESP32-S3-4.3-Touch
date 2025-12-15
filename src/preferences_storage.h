/*
 * Non-volatile storage management for WiFi Radar System
 */

#ifndef PREFERENCES_STORAGE_H
#define PREFERENCES_STORAGE_H

#include <Preferences.h>

// Preferences namespace and keys
extern const char* PREF_NAMESPACE;
extern const char* PREF_KEY_SCAN_SPEED;

// Functions
void saveScanSpeed(uint8_t slider_value);
uint8_t loadScanSpeed(uint8_t default_value = 50);

#endif // PREFERENCES_STORAGE_H



