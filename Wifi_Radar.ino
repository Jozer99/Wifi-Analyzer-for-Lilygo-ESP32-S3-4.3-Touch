/*
 * CRITICAL ESP32-S3 CONFIGURATION:
 * 
 * Before uploading, you MUST configure these settings in Arduino IDE:
 * 
 * 1. Tools > USB CDC On Boot > Enabled
 * 2. Tools > USB Mode > Hardware CDC and JTAG (or "USB CDC")
 * 3. Tools > Flash Size > 8MB (64Mb) - must match your hardware
 * 4. Tools > Board > ESP32 Arduino > ESP32S3 Dev Module (or your specific board)
 * 
 * After uploading:
 * 1. Open Serial Monitor (Tools > Serial Monitor)
 * 2. Set baud rate to 115200
 * 3. Press RESET button on your board
 * 4. You should see output immediately
 * 
 * If you still see no output, try:
 * - Different baud rates: 9600, 115200, 921600
 * - Close and reopen Serial Monitor
 * - Unplug and replug USB cable
 * - Check Device Manager (Windows) to see if COM port is recognized
 */

#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"  // For accessing channel width information

// TEMPORARY: Comment out board library to test if it's causing early crash
// #include <esp_display_panel.hpp>
// using namespace esp_panel::board;

// Configuration: Set to false to skip board initialization (for debugging)
const bool ENABLE_BOARD_INIT = false;

// WiFi scan interval in milliseconds (10 seconds)
const unsigned long SCAN_INTERVAL_MS = 10000;

// Board object for hardware initialization
// Board *board = nullptr;  // Commented out for testing

// Last scan time
unsigned long lastScanTime = 0;

void setup()
{
    // Initialize USB printing function, baud rate is 115200
    // This matches the working I2C example
    Serial.begin(115200);
    
    // Give USB serial time to initialize and Serial Monitor time to connect
    delay(500);
    
    // Use printf() like the working I2C example - it's automatically routed to Serial
    printf("\n\n");
    printf("========================================\r\n");
    printf("WiFi Radar System - Starting\r\n");
    printf("========================================\r\n");
    
    printf("Step 1: Serial communication OK\r\n");
    delay(100);
    
    // Board initialization commented out for testing
    printf("Step 2: Board initialization SKIPPED (testing mode)\r\n");
    delay(100);
    
    // Initialize WiFi in station mode (not connected to any network)
    printf("Step 6: Initializing WiFi...\r\n");
    delay(100);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    printf("Step 7: WiFi initialized in station mode\r\n");
    printf("Ready to scan for networks\r\n");
    printf("========================================\r\n\r\n");
    delay(500);
    
    // Perform initial scan
    printf("Step 8: Starting initial WiFi scan...\r\n");
    performWiFiScan();
    lastScanTime = millis();
    
    printf("Setup complete! Entering main loop...\r\n\r\n");
}

void loop()
{
    // Heartbeat - print every 5 seconds to verify code is running
    static unsigned long lastHeartbeat = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastHeartbeat >= 5000) {
        printf("Heartbeat: System running (uptime: %lu seconds)\r\n", currentTime / 1000);
        lastHeartbeat = currentTime;
    }
    
    // Check if it's time for the next scan
    if (currentTime - lastScanTime >= SCAN_INTERVAL_MS) {
        performWiFiScan();
        lastScanTime = currentTime;
    }
    
    // Small delay to prevent tight loop
    delay(100);
}

// Helper function to get encryption type as string
const char* getEncryptionTypeString(wifi_auth_mode_t encryptionType) {
    switch (encryptionType) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2 Enterprise";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
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
        default: return "Unknown";
    }
}

void performWiFiScan()
{
    printf("\r\n----------------------------------------\r\n");
    printf("Scanning for WiFi networks... (Time: %lu seconds)\r\n", millis() / 1000);
    printf("----------------------------------------\r\n\r\n");
    
    // Use ESP-IDF scan API directly to get channel width information
    wifi_scan_config_t scan_config = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = {
            .active = {
                .min = 100,
                .max = 300
            }
        }
    };
    
    // Start scan using ESP-IDF API
    esp_err_t scan_err = esp_wifi_scan_start(&scan_config, true);  // true = blocking
    if (scan_err != ESP_OK) {
        printf("Error starting WiFi scan: %d\r\n", scan_err);
        return;
    }
    
    // Get scan results
    wifi_ap_record_t ap_records[32];  // Max 32 networks
    uint16_t ap_count = 32;
    esp_err_t err = esp_wifi_scan_get_ap_records(&ap_count, ap_records);
    
    if (err != ESP_OK) {
        printf("Error getting scan records: %d\r\n", err);
        return;
    }
    
    if (ap_count == 0) {
        printf("No networks found.\r\n\r\n");
        return;
    }
    
    // Sort by RSSI (strongest first) - simple bubble sort
    for (uint16_t i = 0; i < ap_count - 1; i++) {
        for (uint16_t j = 0; j < ap_count - i - 1; j++) {
            if (ap_records[j].rssi < ap_records[j + 1].rssi) {
                wifi_ap_record_t temp = ap_records[j];
                ap_records[j] = ap_records[j + 1];
                ap_records[j + 1] = temp;
            }
        }
    }
    
    // Print header
    printf("Found %d network(s):\r\n\r\n", ap_count);
    printf("SSID                          | RSSI  | Ch | Width  | Encryption | BSSID           | Hidden\r\n");
    printf("------------------------------|-------|----|--------|------------|-----------------|-------\r\n");
    
    // Print each network
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
        
        bool isHidden = (ssidLen == 0);
        int rssi = ap_records[i].rssi;
        uint8_t channel = ap_records[i].primary;
        wifi_second_chan_t second = ap_records[i].second;
        wifi_auth_mode_t encryption = ap_records[i].authmode;
        uint8_t* bssid = ap_records[i].bssid;
        
        // Get channel width string
        const char* channelWidth = getChannelWidthString(second);
        
        // Format BSSID (MAC address) as string
        char bssidStr[18];
        sprintf(bssidStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
        
        // Format output with proper spacing using printf
        printf("%-30s | %5d | %2d | %-6s | %-10s | %s | %s\r\n",
               ssidBuf, rssi, channel, channelWidth,
               getEncryptionTypeString(encryption),
               bssidStr,
               isHidden ? "Yes" : "No");
    }
    
    printf("\r\n----------------------------------------\r\n\r\n");
}

