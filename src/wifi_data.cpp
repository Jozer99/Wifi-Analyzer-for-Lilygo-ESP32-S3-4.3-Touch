/*
 * WiFi data structures and graph rendering implementation
 */

#include "wifi_data.h"
#include "config.h"
#include "lvgl_port.h"
#include "wifi_scanner.h"
#include <math.h>
#include <string.h>

// Global WiFi network data (max 64 networks)
WiFiNetworkData wifi_networks[64];
uint16_t wifi_network_count = 0;

// Persistent network storage (for persistence mode)
PersistentNetwork persistent_networks[64];
uint16_t persistent_network_count = 0;

// External UI objects (declared in ui_views.cpp)
extern lv_obj_t *graph_obj;
extern lv_obj_t *vertical_axis_label;
extern lv_obj_t *table_obj;

// Custom draw callback for graph widget - uses Draw Layer API for efficient rendering
void graph_draw_cb(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    
    // Get widget dimensions
    int obj_width = lv_obj_get_width(obj);
    int obj_height = lv_obj_get_height(obj);
    int graph_y_offset = GRAPH_TOP_OFFSET;
    
    // Draw black background
    lv_draw_rect_dsc_t bg_dsc;
    lv_draw_rect_dsc_init(&bg_dsc);
    bg_dsc.bg_opa = LV_OPA_COVER;
    bg_dsc.bg_color = lv_color_hex(0x000000);
    bg_dsc.border_width = 0;
    lv_area_t bg_area = {0, 0, obj_width - 1, obj_height - 1};
    lv_draw_rect(draw_ctx, &bg_dsc, &bg_area);
    
    // Initialize draw descriptors
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    
    // Draw axes (bold lines)
    rect_dsc.bg_color = lv_color_hex(0x444444);
    rect_dsc.bg_opa = LV_OPA_COVER;
    rect_dsc.border_width = 0;
    
    // Left axis (vertical)
    lv_area_t left_axis = {GRAPH_LEFT_MARGIN - 1, graph_y_offset, GRAPH_LEFT_MARGIN, graph_y_offset + GRAPH_HEIGHT - 1};
    lv_draw_rect(draw_ctx, &rect_dsc, &left_axis);
    
    // Bottom axis (horizontal)
    lv_area_t bottom_axis = {GRAPH_LEFT_MARGIN, graph_y_offset + GRAPH_HEIGHT, GRAPH_LEFT_MARGIN + GRAPH_WIDTH - 1, graph_y_offset + GRAPH_HEIGHT + 1};
    lv_draw_rect(draw_ctx, &rect_dsc, &bottom_axis);
    
    // Top border
    lv_area_t top_axis = {GRAPH_LEFT_MARGIN, graph_y_offset, GRAPH_LEFT_MARGIN + GRAPH_WIDTH - 1, graph_y_offset + 1};
    lv_draw_rect(draw_ctx, &rect_dsc, &top_axis);
    
    // Right border
    lv_area_t right_axis = {GRAPH_LEFT_MARGIN + GRAPH_WIDTH, graph_y_offset, GRAPH_LEFT_MARGIN + GRAPH_WIDTH + 1, graph_y_offset + GRAPH_HEIGHT - 1};
    lv_draw_rect(draw_ctx, &rect_dsc, &right_axis);
    
    // Draw RSSI scale labels on left
    label_dsc.font = &lv_font_montserrat_10;
    label_dsc.color = lv_color_hex(0x888888);
    char rssi_label[8];
    for (int rssi = RSSI_MIN; rssi <= RSSI_MAX; rssi += 10) {
        snprintf(rssi_label, sizeof(rssi_label), "%d", rssi);
        int y_pos = graph_y_offset + GRAPH_HEIGHT - ((rssi - RSSI_MIN) * GRAPH_HEIGHT / (RSSI_MAX - RSSI_MIN));
        lv_area_t label_area = {25, y_pos - 5, 75, y_pos + 5};
        lv_draw_label(draw_ctx, &label_dsc, &label_area, rssi_label, NULL);
    }
    
    // Draw channel labels on bottom
    for (int ch = CHANNEL_MIN; ch <= CHANNEL_MAX; ch++) {
        char ch_label[4];
        snprintf(ch_label, sizeof(ch_label), "%d", ch);
        int x_pos = GRAPH_LEFT_MARGIN + ((ch - CHANNEL_MIN) * GRAPH_WIDTH / (CHANNEL_MAX - CHANNEL_MIN));
        
        if ((ch >= 1 && ch <= 11) || ch == 13) {
            label_dsc.color = lv_color_hex(0x888888);
            int label_len = strlen(ch_label);
            int estimated_text_width = label_len * 7;
            if (estimated_text_width < 14) estimated_text_width = 14;
            int text_x_start = x_pos - (estimated_text_width / 2);
            lv_area_t label_area = {text_x_start, graph_y_offset + GRAPH_HEIGHT + 5, text_x_start + estimated_text_width - 1, graph_y_offset + GRAPH_HEIGHT + 20};
            lv_draw_label(draw_ctx, &label_dsc, &label_area, ch_label, NULL);
        }
    }
    
    // Draw horizontal axis title "Wifi Channel"
    label_dsc.color = lv_color_hex(0x888888);
    label_dsc.font = &lv_font_montserrat_12;
    const char* axis_title = "Wifi Channel";
    int title_width = strlen(axis_title) * 8;
    int horizontal_title_x = GRAPH_LEFT_MARGIN + (GRAPH_WIDTH / 2) - (title_width / 2);
    lv_area_t title_area = {horizontal_title_x, graph_y_offset + GRAPH_HEIGHT + 18, horizontal_title_x + title_width - 1, graph_y_offset + GRAPH_HEIGHT + 35};
    lv_draw_label(draw_ctx, &label_dsc, &title_area, axis_title, NULL);
    
    // Vertical axis title "RSSI (dB)" is now handled by a rotated label widget
    // (created in main.cpp, no need to draw it here)
    
    // Draw gridlines using dashed lines (much more efficient!)
    line_dsc.color = lv_color_hex(0x333333);
    line_dsc.width = 1;
    line_dsc.opa = LV_OPA_COVER;
    line_dsc.dash_width = 2;  // 2 pixel dashes
    line_dsc.dash_gap = 2;    // 2 pixel gaps
    
    // Draw vertical gridlines for channels 0-15
    for (int ch = 0; ch <= 15; ch++) {
        int x_pos = GRAPH_LEFT_MARGIN + ((ch - CHANNEL_MIN) * GRAPH_WIDTH / (CHANNEL_MAX - CHANNEL_MIN));
        lv_point_t p1 = {x_pos, graph_y_offset};
        lv_point_t p2 = {x_pos, graph_y_offset + GRAPH_HEIGHT};
        lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
    }
    
    // Draw horizontal gridlines every 10 dB
    for (int rssi = RSSI_MIN; rssi <= RSSI_MAX; rssi += 10) {
        int y_pos = graph_y_offset + GRAPH_HEIGHT - ((rssi - RSSI_MIN) * GRAPH_HEIGHT / (RSSI_MAX - RSSI_MIN));
        lv_point_t p1 = {GRAPH_LEFT_MARGIN, y_pos};
        lv_point_t p2 = {GRAPH_LEFT_MARGIN + GRAPH_WIDTH, y_pos};
        lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
    }
    
    // Draw WiFi networks as half-ovals using rectangles with transparency
    const lv_color_t network_palette[] = {
        lv_color_hex(0x4a6670), lv_color_hex(0x668f80), lv_color_hex(0xa0af84),
        lv_color_hex(0xc3b59f), lv_color_hex(0xd6a2ad)
    };
    const uint8_t palette_size = sizeof(network_palette) / sizeof(network_palette[0]);
    
    for (uint16_t i = 0; i < wifi_network_count; i++) {
        WiFiNetworkData *net = &wifi_networks[i];
        
        int oval_height = net->y_bottom - net->y_top;
        if (oval_height <= 0) continue;
        
        // Draw filled half-oval with transparency using rectangles with opacity
        rect_dsc.bg_color = net->color;
        rect_dsc.bg_opa = LV_OPA_10;  // 10% opacity (built-in support!)
        rect_dsc.border_width = 0;
        
        // Draw the fill using horizontal rectangles (parabolic shape)
        for (int y = net->y_top; y <= net->y_bottom; y++) {
            float progress = (float)(y - net->y_top) / (float)oval_height;
            float width_factor = sqrt(progress);
            int width_at_y = (int)(net->width_pixels * width_factor);
            int x_start = net->x_center - width_at_y / 2;
            int x_end = net->x_center + width_at_y / 2;
            
            // Clamp to graph bounds
            if (x_start < GRAPH_LEFT_MARGIN) x_start = GRAPH_LEFT_MARGIN;
            if (x_end > GRAPH_LEFT_MARGIN + GRAPH_WIDTH) x_end = GRAPH_LEFT_MARGIN + GRAPH_WIDTH;
            if (x_end <= x_start) continue;
            
            lv_area_t fill_area = {x_start, y, x_end - 1, y};
            lv_draw_rect(draw_ctx, &rect_dsc, &fill_area);
        }
        
        // Draw outline with lines
        line_dsc.color = net->color;
        line_dsc.width = 2;
        line_dsc.opa = LV_OPA_COVER;
        line_dsc.dash_width = 0;  // Solid line for outline
        line_dsc.dash_gap = 0;
        
        // Draw left edge (curved)
        int prev_x = -1, prev_y = -1;
        for (int y = net->y_top; y <= net->y_bottom; y++) {
            float progress = (float)(y - net->y_top) / (float)oval_height;
            float width_factor = sqrt(progress);
            int width_at_y = (int)(net->width_pixels * width_factor);
            int x_edge = net->x_center - width_at_y / 2;
            
            if (prev_x >= 0 && prev_y >= 0) {
                lv_point_t p1 = {prev_x, prev_y};
                lv_point_t p2 = {x_edge, y};
                lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
            }
            prev_x = x_edge;
            prev_y = y;
        }
        
        // Draw right edge (curved)
        prev_x = -1;
        prev_y = -1;
        for (int y = net->y_top; y <= net->y_bottom; y++) {
            float progress = (float)(y - net->y_top) / (float)oval_height;
            float width_factor = sqrt(progress);
            int width_at_y = (int)(net->width_pixels * width_factor);
            int x_edge = net->x_center + width_at_y / 2;
            
            if (prev_x >= 0 && prev_y >= 0) {
                lv_point_t p1 = {prev_x, prev_y};
                lv_point_t p2 = {x_edge, y};
                lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
            }
            prev_x = x_edge;
            prev_y = y;
        }
        
        // Draw bottom edge (straight line)
        int x_start = net->x_center - net->width_pixels / 2;
        int x_end = net->x_center + net->width_pixels / 2;
        if (x_start < GRAPH_LEFT_MARGIN) x_start = GRAPH_LEFT_MARGIN;
        if (x_end > GRAPH_LEFT_MARGIN + GRAPH_WIDTH) x_end = GRAPH_LEFT_MARGIN + GRAPH_WIDTH;
        if (x_end > x_start) {
            lv_point_t p1 = {x_start, net->y_bottom};
            lv_point_t p2 = {x_end, net->y_bottom};
            lv_draw_line(draw_ctx, &line_dsc, &p1, &p2);
        }
        
        // Draw SSID label above the network
        label_dsc.color = net->color;
        label_dsc.font = &lv_font_montserrat_10;
        label_dsc.opa = LV_OPA_COVER;
        int text_len = strlen(net->ssid);
        int estimated_text_width = text_len * 7;
        if (estimated_text_width < 50) estimated_text_width = 50;
        int text_x_start = net->x_center - (estimated_text_width / 2);
        if (text_x_start < 0) text_x_start = 0;
        if (text_x_start + estimated_text_width > obj_width) {
            text_x_start = obj_width - estimated_text_width;
            if (text_x_start < 0) {
                text_x_start = 0;
                estimated_text_width = obj_width;
            }
        }
        lv_area_t ssid_area = {text_x_start, net->y_top - 15, text_x_start + estimated_text_width - 1, net->y_top};
        lv_draw_label(draw_ctx, &label_dsc, &ssid_area, net->ssid, NULL);
    }
}

// Update the WiFi graph on screen - now just stores data and invalidates the widget
void updateWiFiGraph(wifi_ap_record_t *ap_records, uint16_t ap_count) {
    if (graph_obj == NULL) return;
    
    lvgl_port_lock(-1);
    
    // Limit to max 64 networks
    if (ap_count > 64) ap_count = 64;
    wifi_network_count = ap_count;
    
    // Color palette for networks
    const lv_color_t network_palette[] = {
        lv_color_hex(0x4a6670), lv_color_hex(0x668f80), lv_color_hex(0xa0af84),
        lv_color_hex(0xc3b59f), lv_color_hex(0xd6a2ad)
    };
    const uint8_t palette_size = sizeof(network_palette) / sizeof(network_palette[0]);
    
    int graph_y_offset = GRAPH_TOP_OFFSET;
    
    // Store network data for draw callback
    for (uint16_t i = 0; i < ap_count; i++) {
        WiFiNetworkData *net = &wifi_networks[i];
        
        int rssi = ap_records[i].rssi;
        uint8_t channel = ap_records[i].primary;
        wifi_second_chan_t second = ap_records[i].second;
        
        // Clamp RSSI to valid range
        if (rssi < RSSI_MIN) rssi = RSSI_MIN;
        if (rssi > RSSI_MAX) rssi = RSSI_MAX;
        
        // Calculate center channel and width
        float center_channel = channel;
        int width_channels = 4;  // Default 20MHz = 4 channels
        
        if (second == WIFI_SECOND_CHAN_ABOVE) {
            center_channel = channel + 2.0f;
            width_channels = 8;
        } else if (second == WIFI_SECOND_CHAN_BELOW) {
            center_channel = channel - 2.0f;
            width_channels = 8;
        }
        
        // Calculate positions
        net->x_center = GRAPH_LEFT_MARGIN + (int)((center_channel - CHANNEL_MIN) * GRAPH_WIDTH / (float)(CHANNEL_MAX - CHANNEL_MIN));
        net->width_pixels = (width_channels * GRAPH_WIDTH / (CHANNEL_MAX - CHANNEL_MIN));
        net->y_bottom = graph_y_offset + GRAPH_HEIGHT;
        net->y_top = graph_y_offset + GRAPH_HEIGHT - ((rssi - RSSI_MIN) * GRAPH_HEIGHT / (RSSI_MAX - RSSI_MIN));
        
        // Store network properties
        net->rssi = rssi;
        net->channel = channel;
        net->second = second;
        net->center_channel = center_channel;
        net->width_channels = width_channels;
        net->color = network_palette[i % palette_size];
        
        // Store SSID
        int ssidLen = strlen((char*)ap_records[i].ssid);
        if (ssidLen == 0 || ssidLen > 32) {
            strcpy(net->ssid, "(hidden)");
        } else {
            memcpy(net->ssid, ap_records[i].ssid, ssidLen);
            net->ssid[ssidLen] = '\0';
        }
    }
    
    // Invalidate the widget to trigger redraw
    lv_obj_invalidate(graph_obj);
    
    lvgl_port_unlock();
}

// Update the WiFi table view
void updateWiFiTable(wifi_ap_record_t *ap_records, uint16_t ap_count) {
    if (table_obj == NULL) return;
    
    lvgl_port_lock(-1);
    
    // Limit to max 64 networks
    if (ap_count > 64) ap_count = 64;
    
    // Set row count: only data rows (no header row in table)
    lv_table_set_row_cnt(table_obj, ap_count);
    
    // Set column count
    lv_table_set_col_cnt(table_obj, 5);
    
    // Set column widths (in pixels) - maximize use of 640px width
    // Total available: 640px - 10px padding = 630px
    // "WPA2 Enterprise" needs ~140px, RSSI and Width need more space
    lv_table_set_col_width(table_obj, 0, 220);  // SSID (slightly wider)
    lv_table_set_col_width(table_obj, 1, 60);    // Channel
    lv_table_set_col_width(table_obj, 2, 85);    // RSSI (wider)
    lv_table_set_col_width(table_obj, 3, 95);    // Width (wider)
    lv_table_set_col_width(table_obj, 4, 150);   // Security (wide enough for "WPA2 Enterprise")
    // Total: 220 + 60 + 85 + 95 + 150 = 610px (leaves 20px for scrollbar/margins)
    
    // Populate data rows (no header row - it's fixed above)
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
        
        // Truncate SSID if too long
        if (strlen(ssidBuf) > 25) {
            ssidBuf[25] = '\0';
        }
        
        int rssi = ap_records[i].rssi;
        uint8_t channel = ap_records[i].primary;
        wifi_second_chan_t second = ap_records[i].second;
        wifi_auth_mode_t encryption = ap_records[i].authmode;
        
        char rssiStr[8];
        char chStr[4];
        snprintf(rssiStr, sizeof(rssiStr), "%d", rssi);
        snprintf(chStr, sizeof(chStr), "%d", channel);
        
        // Set table cell values (row i, no header row in table)
        lv_table_set_cell_value(table_obj, i, 0, ssidBuf);
        lv_table_set_cell_value(table_obj, i, 1, chStr);
        lv_table_set_cell_value(table_obj, i, 2, rssiStr);
        lv_table_set_cell_value(table_obj, i, 3, getChannelWidthString(second));
        lv_table_set_cell_value(table_obj, i, 4, getEncryptionTypeString(encryption));
    }
    
    lvgl_port_unlock();
}

// Helper function to compare BSSIDs (MAC addresses)
static bool compareBSSID(uint8_t *bssid1, uint8_t *bssid2) {
    for (int i = 0; i < 6; i++) {
        if (bssid1[i] != bssid2[i]) {
            return false;
        }
    }
    return true;
}

// Clear all persistent networks
void clearPersistentNetworks() {
    persistent_network_count = 0;
    for (int i = 0; i < 64; i++) {
        persistent_networks[i].valid = false;
    }
}

// Merge scan results with persistent network list
// When persistence mode is enabled, this function:
// - Adds new networks to the persistent list
// - Updates existing networks (keeps max RSSI)
// - Keeps networks not found in current scan
void mergeScanResultsWithPersistent(wifi_ap_record_t *ap_records, uint16_t ap_count, wifi_ap_record_t *merged_records, uint16_t *merged_count) {
    extern bool persistence_enabled;
    
    // If persistence is disabled, just copy scan results directly
    if (!persistence_enabled) {
        if (ap_count > 64) ap_count = 64;
        for (uint16_t i = 0; i < ap_count; i++) {
            merged_records[i] = ap_records[i];
        }
        *merged_count = ap_count;
        return;
    }
    
    // Persistence mode: merge scan results with persistent list
    
    // Helper function to sort persistent networks by RSSI (strongest first)
    auto sortPersistentNetworks = []() {
        for (uint16_t i = 0; i < persistent_network_count - 1; i++) {
            for (uint16_t j = 0; j < persistent_network_count - i - 1; j++) {
                if (!persistent_networks[j].valid || !persistent_networks[j + 1].valid) continue;
                if (persistent_networks[j].record.rssi < persistent_networks[j + 1].record.rssi) {
                    PersistentNetwork temp = persistent_networks[j];
                    persistent_networks[j] = persistent_networks[j + 1];
                    persistent_networks[j + 1] = temp;
                }
            }
        }
    };
    
    // Helper function to find the index of the weakest network
    auto findWeakestNetworkIndex = []() -> int {
        int weakest_idx = -1;
        int weakest_rssi = 0;  // Start with a value that will be replaced
        
        for (uint16_t i = 0; i < persistent_network_count; i++) {
            if (persistent_networks[i].valid) {
                if (weakest_idx == -1 || persistent_networks[i].record.rssi < weakest_rssi) {
                    weakest_idx = i;
                    weakest_rssi = persistent_networks[i].record.rssi;
                }
            }
        }
        return weakest_idx;
    };
    
    // Step 1: Update existing persistent networks or add new ones from scan
    for (uint16_t i = 0; i < ap_count; i++) {
        bool found = false;
        
        // Check if this network already exists in persistent list (by BSSID)
        for (uint16_t j = 0; j < persistent_network_count; j++) {
            if (persistent_networks[j].valid && 
                compareBSSID(persistent_networks[j].record.bssid, ap_records[i].bssid)) {
                // Network exists - update to keep max RSSI
                int old_rssi = persistent_networks[j].record.rssi;
                // Update all fields from current scan
                persistent_networks[j].record = ap_records[i];
                // Keep maximum RSSI value
                if (old_rssi > ap_records[i].rssi) {
                    persistent_networks[j].record.rssi = old_rssi;
                }
                found = true;
                break;
            }
        }
        
        // If not found, add as new network
        if (!found) {
            if (persistent_network_count < 64) {
                // Room for new network
                persistent_networks[persistent_network_count].record = ap_records[i];
                persistent_networks[persistent_network_count].valid = true;
                persistent_network_count++;
            } else {
                // At capacity (64 networks) - find weakest and check if new network is stronger
                int weakest_idx = findWeakestNetworkIndex();
                if (weakest_idx >= 0 && 
                    ap_records[i].rssi > persistent_networks[weakest_idx].record.rssi) {
                    // Replace weakest with new network
                    persistent_networks[weakest_idx].record = ap_records[i];
                    persistent_networks[weakest_idx].valid = true;
                }
                // If new network is weaker or equal, ignore it
            }
        }
    }
    
    // Step 2: Sort persistent networks by RSSI (strongest first)
    sortPersistentNetworks();
    
    // Step 3: Copy all persistent networks to merged_records (already sorted)
    *merged_count = 0;
    for (uint16_t i = 0; i < persistent_network_count && *merged_count < 64; i++) {
        if (persistent_networks[i].valid) {
            merged_records[*merged_count] = persistent_networks[i].record;
            (*merged_count)++;
        }
    }
    
    // merged_records is already sorted by RSSI (strongest first) since persistent_networks is sorted
}

