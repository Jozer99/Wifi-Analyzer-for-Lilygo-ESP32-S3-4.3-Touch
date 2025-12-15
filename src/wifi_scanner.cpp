/*
 * WiFi scanning functionality implementation
 */

#include "wifi_scanner.h"
#include "wifi_data.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>
#include <string.h>

// Static buffers for scan results (to avoid stack overflow)
static wifi_ap_record_t scan_ap_records[64];
static wifi_ap_record_t scan_merged_records[64];

// WiFi scan time per channel in milliseconds (0.25s to 2s, default 1.125s)
uint16_t scan_time_per_channel_ms = 1125;  // Default to middle value

// Helper function to get encryption type as string
const char* getEncryptionTypeString(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 Ent";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/3";
        case WIFI_AUTH_WAPI_PSK: return "WAPI";
        default: return "Unknown";
    }
}

// Helper function to get channel width as string
const char* getChannelWidthString(wifi_second_chan_t secondChannel) {
    switch (secondChannel) {
        case WIFI_SECOND_CHAN_NONE: return "20MHz";
        case WIFI_SECOND_CHAN_ABOVE: return "40MHz+";
        case WIFI_SECOND_CHAN_BELOW: return "40MHz-";
        default: return "?";
    }
}

// Debug function: Print WiFi networks table to serial terminal
void printWiFiTableDebug(wifi_ap_record_t *ap_records, uint16_t ap_count) {
    Serial.println("\r\n");
    Serial.println("==================================================================================");
    Serial.println("WiFi Networks (sorted by signal strength)");
    Serial.println("==================================================================================");
    Serial.printf("%-32s %6s %6s %12s %-12s\r\n", "SSID", "RSSI", "Channel", "Channel Width", "Encryption");
    Serial.println("----------------------------------------------------------------------------------");
    
    for (uint16_t i = 0; i < ap_count; i++) {
        // Extract SSID
        char ssidBuf[33];
        int ssidLen = strlen((char*)ap_records[i].ssid);
        if (ssidLen == 0 || ssidLen > 32) {
            strcpy(ssidBuf, "(hidden)");
        } else {
            memcpy(ssidBuf, ap_records[i].ssid, ssidLen);
            ssidBuf[ssidLen] = '\0';
        }
        
        int rssi = ap_records[i].rssi;
        uint8_t channel = ap_records[i].primary;
        wifi_second_chan_t second = ap_records[i].second;
        wifi_auth_mode_t encryption = ap_records[i].authmode;
        
        Serial.printf("%-32s %6d %6d %12s %-12s\r\n", 
                      ssidBuf, 
                      rssi, 
                      channel, 
                      getChannelWidthString(second),
                      getEncryptionTypeString(encryption));
    }
    
    Serial.println("==================================================================================");
    Serial.println("\r\n");
}

void performWiFiScan()
{
    Serial.printf("\r\nScanning for WiFi networks... (Time: %lu seconds)\r\n", millis() / 1000);
    
    // Configure scan parameters
    wifi_scan_config_t scan_config = {};
    scan_config.ssid = NULL;
    scan_config.bssid = NULL;
    scan_config.channel = 0;  // 0 = scan all channels
    scan_config.show_hidden = true;
    scan_config.scan_type = WIFI_SCAN_TYPE_ACTIVE;
    // Use the configurable scan time per channel (values are in milliseconds)
    // Minimum is 120ms (ESP-IDF requirement), maximum is 2000ms
    uint16_t scan_time_ms = scan_time_per_channel_ms;
    if (scan_time_ms < 120) scan_time_ms = 120;  // ESP-IDF minimum
    if (scan_time_ms > 2000) scan_time_ms = 2000;
    scan_config.scan_time.active.min = scan_time_ms;
    scan_config.scan_time.active.max = scan_time_ms;  // Set both to same value for fixed duration
    scan_config.scan_time.passive = scan_time_ms;
    
    // Record start time
    unsigned long scan_start_time = millis();
    
    // Start scan using ESP-IDF API (blocking mode)
    // Note: In blocking mode, this should wait for the scan to complete
    esp_err_t scan_err = esp_wifi_scan_start(&scan_config, true);
    
    // Record time after scan_start returns
    unsigned long scan_end_time = millis();
    unsigned long scan_duration = scan_end_time - scan_start_time;
    
    // Expected time: scan_time_ms * number_of_channels (typically 14 for 2.4GHz)
    unsigned long expected_time = (unsigned long)scan_time_ms * 14;
    Serial.printf("Scan: %d ms/channel, actual=%lu ms, expected=%lu ms\r\n", 
                  scan_time_ms, scan_duration, expected_time);
    
    if (scan_err != ESP_OK) {
        Serial.printf("Scan failed with error: %d\r\n", scan_err);
        return;
    }
    
    // Retrieve scan results (use static buffer to avoid stack overflow)
    uint16_t ap_count = 64;
    esp_err_t err = esp_wifi_scan_get_ap_records(&ap_count, scan_ap_records);
    
    if (err != ESP_OK || ap_count == 0) {
        Serial.println("No networks found.");
        return;
    }
    
    // Sort by RSSI (strongest first)
    for (uint16_t i = 0; i < ap_count - 1; i++) {
        for (uint16_t j = 0; j < ap_count - i - 1; j++) {
            if (scan_ap_records[j].rssi < scan_ap_records[j + 1].rssi) {
                wifi_ap_record_t temp = scan_ap_records[j];
                scan_ap_records[j] = scan_ap_records[j + 1];
                scan_ap_records[j + 1] = temp;
            }
        }
    }
    
    Serial.printf("Found %d network(s)\r\n", ap_count);
    
    // Merge scan results with persistent list (if persistence mode is enabled)
    // Use static buffer to avoid stack overflow
    uint16_t merged_count = 0;
    mergeScanResultsWithPersistent(scan_ap_records, ap_count, scan_merged_records, &merged_count);
    
    // Print debug table to serial (use merged results)
    printWiFiTableDebug(scan_merged_records, merged_count);
    
    // Update the display graph and table (use merged results)
    updateWiFiGraph(scan_merged_records, merged_count);
    updateWiFiTable(scan_merged_records, merged_count);
}

