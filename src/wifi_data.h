/*
 * WiFi data structures and graph rendering
 */

#ifndef WIFI_DATA_H
#define WIFI_DATA_H

#include <lvgl.h>
#include "esp_wifi.h"

// WiFi network data structure for draw callback
struct WiFiNetworkData {
    int rssi;
    uint8_t channel;
    wifi_second_chan_t second;
    float center_channel;
    int width_channels;
    lv_color_t color;
    char ssid[33];
    int x_center;
    int y_top;
    int y_bottom;
    int width_pixels;
};

// Global WiFi network data (extern declarations)
extern WiFiNetworkData wifi_networks[64];
extern uint16_t wifi_network_count;

// Global UI objects
extern lv_obj_t *vertical_axis_label;  // Rotated label for "RSSI (dB)" title

// Persistent network storage (for persistence mode)
struct PersistentNetwork {
    wifi_ap_record_t record;
    bool valid;  // true if this slot contains a valid network
};

// Functions
void graph_draw_cb(lv_event_t *e);
void updateWiFiGraph(wifi_ap_record_t *ap_records, uint16_t ap_count);
void updateWiFiTable(wifi_ap_record_t *ap_records, uint16_t ap_count);
void mergeScanResultsWithPersistent(wifi_ap_record_t *ap_records, uint16_t ap_count, wifi_ap_record_t *merged_records, uint16_t *merged_count);
void clearPersistentNetworks();

#endif // WIFI_DATA_H

