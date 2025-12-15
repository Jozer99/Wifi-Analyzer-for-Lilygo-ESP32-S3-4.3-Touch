/*
 * WiFi scanning functionality
 */

#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include "esp_wifi.h"

// Helper functions
const char* getEncryptionTypeString(wifi_auth_mode_t encryptionType);
const char* getChannelWidthString(wifi_second_chan_t secondChannel);
void printWiFiTableDebug(wifi_ap_record_t *ap_records, uint16_t ap_count);

// Main scanning function
void performWiFiScan();

// Scan time configuration (extern)
extern uint16_t scan_time_per_channel_ms;

#endif // WIFI_SCANNER_H



