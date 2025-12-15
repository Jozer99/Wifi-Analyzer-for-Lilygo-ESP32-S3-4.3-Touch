/*
 * WiFi Radar System with Visual Display
 * 
 * Main entry point - orchestrates initialization and main loop
 */

#include <Arduino.h>
#include <WiFi.h>
#include "esp_wifi.h"
#include <lvgl.h>
#include "config.h"
#include "lvgl_port.h"
#include "wifi_scanner.h"
#include "ui_views.h"
#include "ui_handlers.h"
#include "wifi_data.h"  // For graph_draw_cb

// Global state
bool scanning_paused = false;
bool persistence_enabled = false;  // Persistence mode: maintain growing list of networks
unsigned long lastScanTime = 0;

void setup()
{
    Serial.begin(115200);
    delay(500);
    
    printf("\n\n");
    printf("========================================\r\n");
    printf("WiFi Radar System - Visual Display\r\n");
    printf("========================================\r\n");
    
    // Initialize WiFi
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();
    delay(100);
    esp_wifi_disconnect();
    delay(100);
    
    printf("WiFi initialized in station mode\r\n");
    
    // Initialize LVGL and display
    lvgl_port_init();
    
    /* Lock the mutex due to the LVGL APIs are not thread-safe */
    lvgl_port_lock(-1);
    
    // Create main screen
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000000), LV_PART_MAIN);
    
    // Create info window container (left region: 640x480)
    info_window = lv_obj_create(scr);
    lv_obj_set_size(info_window, INFO_WINDOW_WIDTH, INFO_WINDOW_HEIGHT);
    lv_obj_align(info_window, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(info_window, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(info_window, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(info_window, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(info_window, 0, LV_PART_MAIN);
    
    // Create graph object for WiFi visualization (using custom draw callback)
    graph_obj = lv_obj_create(info_window);
    lv_obj_set_size(graph_obj, INFO_WINDOW_WIDTH, INFO_WINDOW_HEIGHT);
    lv_obj_align(graph_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(graph_obj, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(graph_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(graph_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(graph_obj, 0, LV_PART_MAIN);
    lv_obj_add_event_cb(graph_obj, graph_draw_cb, LV_EVENT_DRAW_MAIN, NULL);
    
    // Create rotated label for vertical axis title "RSSI (dB)"
    vertical_axis_label = lv_label_create(info_window);
    lv_label_set_text(vertical_axis_label, "RSSI (dB)");
    lv_obj_set_style_text_color(vertical_axis_label, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(vertical_axis_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(vertical_axis_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(vertical_axis_label, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(vertical_axis_label, 0, LV_PART_MAIN);
    lv_obj_clear_flag(vertical_axis_label, LV_OBJ_FLAG_CLICKABLE);
    
    // Update layout to get actual label dimensions before rotation
    lv_obj_update_layout(vertical_axis_label);
    
    // Get label dimensions and set rotation pivot to center
    uint32_t lbl_h = lv_obj_get_height(vertical_axis_label);
    uint32_t lbl_w = lv_obj_get_width(vertical_axis_label);
    lv_obj_set_style_transform_pivot_x(vertical_axis_label, lbl_w / 2, LV_PART_MAIN);
    lv_obj_set_style_transform_pivot_y(vertical_axis_label, lbl_h / 2, LV_PART_MAIN);
    
    // Rotate -90 degrees (counterclockwise) - LVGL 8.x uses transform_angle in 0.1 degree units, so -90° = -900
    lv_obj_set_style_transform_angle(vertical_axis_label, -900, LV_PART_MAIN);
    
    // Position label on the left side, centered vertically on the graph
    // After -90° rotation, the original width becomes the vertical extent
    // RSSI scale labels are drawn at x=25, so position title well to the left
    int graph_center_y = GRAPH_TOP_OFFSET + (GRAPH_HEIGHT / 2);
    // After rotation, lbl_w (original width) becomes the height, so center using half of that
    // After rotation, lbl_h (original height) becomes the width
    // Use larger negative x to account for rotated label's bounding box and move it to the very left edge
    // Try -lbl_h to fully compensate for the rotated width, or even more if needed
    lv_obj_set_pos(vertical_axis_label, -lbl_h, graph_center_y - (lbl_w / 2));
    
    // Update layout after positioning to ensure transform is applied
    lv_obj_update_layout(vertical_axis_label);
    
    // Move label to front to ensure it's visible above graph
    lv_obj_move_foreground(vertical_axis_label);
    
    // Create table view
    createTableView();
    
    // Create settings view
    createSettingsView();
    
    // Create menu bar (right region: 160x480)
    createMenuBar(scr);
    
    // Set initial view to graph
    switchToGraphView(NULL);
    
    /* Release the mutex */
    lvgl_port_unlock();
    
    printf("Display initialized\r\n");
    printf("Ready to scan for networks\r\n");
    printf("========================================\r\n\r\n");
    
    // Perform initial scan
    performWiFiScan();
    lastScanTime = millis();
    
    printf("Setup complete! Entering main loop...\r\n\r\n");
}

void loop()
{
    unsigned long currentTime = millis();
    
    // Check if it's time for the next scan (only if not paused)
    if (!scanning_paused && (currentTime - lastScanTime >= SCAN_INTERVAL_MS)) {
        performWiFiScan();
        lastScanTime = currentTime;
    }
    
    // Small delay to prevent tight loop
    delay(100);
}
