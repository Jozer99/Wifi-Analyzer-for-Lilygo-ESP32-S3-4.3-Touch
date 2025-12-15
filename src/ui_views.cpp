/*
 * UI view creation implementation
 */

#include "ui_views.h"
#include "ui_handlers.h"
#include "config.h"
#include "preferences_storage.h"
#include "wifi_scanner.h"
#include "lvgl_port.h"
#include <Arduino.h>

// Global UI objects
lv_obj_t *graph_obj = NULL;
lv_obj_t *vertical_axis_label = NULL;
lv_obj_t *table_obj = NULL;
lv_obj_t *table_header = NULL;
lv_obj_t *settings_obj = NULL;
lv_obj_t *info_window = NULL;
lv_obj_t *menu_bar = NULL;
lv_obj_t *pause_btn = NULL;
lv_obj_t *persistence_btn = NULL;
lv_obj_t *graph_btn = NULL;
lv_obj_t *table_btn = NULL;
lv_obj_t *settings_btn = NULL;

// Create menu bar
void createMenuBar(lv_obj_t *parent) {
    // Create menu bar container
    menu_bar = lv_obj_create(parent);
    lv_obj_set_size(menu_bar, MENU_BAR_WIDTH, MENU_BAR_HEIGHT);
    lv_obj_align(menu_bar, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(menu_bar, lv_color_hex(0x252526), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(menu_bar, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(menu_bar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(menu_bar, 0, LV_PART_MAIN);  // Square corners
    lv_obj_set_style_pad_all(menu_bar, 5, LV_PART_MAIN);
    lv_obj_set_flex_flow(menu_bar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menu_bar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(menu_bar, 5, LV_PART_MAIN);
    // Disable scrolling to prevent scrollbar
    lv_obj_set_scrollbar_mode(menu_bar, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(menu_bar, LV_OBJ_FLAG_SCROLLABLE);
    
    // Calculate button height to evenly distribute 5 buttons across menu bar
    // Menu bar: 480px total
    // - Top/bottom padding: 10px (5px each)
    // - 4 gaps between 5 buttons: 20px (5px each)
    // - 5 buttons: 5 * button_height
    // So: 10 + 20 + 5*button_height = 480
    // Therefore: button_height = (480 - 30) / 5 = 90px
    int button_height = (MENU_BAR_HEIGHT - 10 - 20) / 5;
    
    // Helper function to style buttons with common settings
    auto styleButton = [](lv_obj_t *btn) {
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x2d2d30), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x007acc), LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(btn, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_CHECKED);
    };
    
    // Pause button
    pause_btn = lv_btn_create(menu_bar);
    lv_obj_set_size(pause_btn, MENU_BAR_WIDTH - 10, button_height);
    styleButton(pause_btn);
    lv_obj_t *pause_label = lv_label_create(pause_btn);
    lv_label_set_text(pause_label, "Pause");
    lv_obj_set_style_text_font(pause_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(pause_label);
    lv_obj_add_event_cb(pause_btn, togglePause, LV_EVENT_CLICKED, NULL);
    
    // Persistence button
    persistence_btn = lv_btn_create(menu_bar);
    lv_obj_set_size(persistence_btn, MENU_BAR_WIDTH - 10, button_height);
    styleButton(persistence_btn);
    lv_obj_t *persistence_label = lv_label_create(persistence_btn);
    lv_label_set_text(persistence_label, "Persistence");
    lv_obj_set_style_text_font(persistence_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(persistence_label);
    lv_obj_add_event_cb(persistence_btn, togglePersistence, LV_EVENT_CLICKED, NULL);
    
    // Graph view button
    graph_btn = lv_btn_create(menu_bar);
    lv_obj_set_size(graph_btn, MENU_BAR_WIDTH - 10, button_height);
    styleButton(graph_btn);
    lv_obj_add_state(graph_btn, LV_STATE_CHECKED);  // Default view
    lv_obj_t *graph_label = lv_label_create(graph_btn);
    lv_label_set_text(graph_label, "Graph");
    lv_obj_set_style_text_font(graph_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(graph_label);
    lv_obj_add_event_cb(graph_btn, switchToGraphView, LV_EVENT_CLICKED, NULL);
    
    // Table view button
    table_btn = lv_btn_create(menu_bar);
    lv_obj_set_size(table_btn, MENU_BAR_WIDTH - 10, button_height);
    styleButton(table_btn);
    lv_obj_t *table_label = lv_label_create(table_btn);
    lv_label_set_text(table_label, "Table");
    lv_obj_set_style_text_font(table_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(table_label);
    lv_obj_add_event_cb(table_btn, switchToTableView, LV_EVENT_CLICKED, NULL);
    
    // Settings view button
    settings_btn = lv_btn_create(menu_bar);
    lv_obj_set_size(settings_btn, MENU_BAR_WIDTH - 10, button_height);
    styleButton(settings_btn);
    lv_obj_t *settings_label = lv_label_create(settings_btn);
    lv_label_set_text(settings_label, "Settings");
    lv_obj_set_style_text_font(settings_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(settings_label);
    lv_obj_add_event_cb(settings_btn, switchToSettingsView, LV_EVENT_CLICKED, NULL);
}

// Create table view
void createTableView() {
    if (info_window == NULL) return;
    
    // Create fixed header row container
    table_header = lv_obj_create(info_window);
    lv_obj_set_size(table_header, INFO_WINDOW_WIDTH, 30);
    lv_obj_align(table_header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(table_header, lv_color_hex(0x252526), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(table_header, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(table_header, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(table_header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(table_header, 5, LV_PART_MAIN);
    lv_obj_clear_flag(table_header, LV_OBJ_FLAG_SCROLLABLE);
    
    // Use flex layout to match table column widths exactly
    lv_obj_set_flex_flow(table_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(table_header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(table_header, 0, LV_PART_MAIN);
    
    // Column widths matching the table: 220, 60, 85, 95, 150
    int col_widths[] = {220, 60, 85, 95, 150};
    const char* header_texts[] = {"SSID", "Ch", "RSSI", "Width", "Security"};
    
    for (int i = 0; i < 5; i++) {
        lv_obj_t *header_label = lv_label_create(table_header);
        lv_label_set_text(header_label, header_texts[i]);
        lv_obj_set_size(header_label, col_widths[i], LV_SIZE_CONTENT);
        lv_obj_set_style_text_color(header_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_set_style_text_font(header_label, &lv_font_montserrat_14, LV_PART_MAIN);
        lv_obj_set_style_pad_all(header_label, 4, LV_PART_MAIN);
    }
    
    // Create scrollable table below header
    table_obj = lv_table_create(info_window);
    lv_obj_set_size(table_obj, INFO_WINDOW_WIDTH, INFO_WINDOW_HEIGHT - 30);
    lv_obj_align(table_obj, LV_ALIGN_TOP_LEFT, 0, 30);
    lv_obj_set_style_bg_color(table_obj, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(table_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(table_obj, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(table_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(table_obj, 5, LV_PART_MAIN);
    
    // Set cell background and text colors
    lv_obj_set_style_bg_color(table_obj, lv_color_hex(0x000000), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(table_obj, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_text_color(table_obj, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);
    lv_obj_set_style_border_color(table_obj, lv_color_hex(0x333333), LV_PART_ITEMS);
    lv_obj_set_style_border_width(table_obj, 1, LV_PART_ITEMS);
    lv_obj_set_style_pad_all(table_obj, 4, LV_PART_ITEMS);
    lv_obj_set_style_pad_top(table_obj, 8, LV_PART_ITEMS);
    lv_obj_set_style_pad_bottom(table_obj, 8, LV_PART_ITEMS);
    
    // Make table scrollable
    lv_obj_set_scroll_dir(table_obj, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(table_obj, LV_SCROLLBAR_MODE_AUTO);
    
    // Initially hidden (graph is default view)
    lv_obj_add_flag(table_obj, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(table_header, LV_OBJ_FLAG_HIDDEN);
}

// Create settings view
void createSettingsView() {
    if (info_window == NULL) return;
    
    settings_obj = lv_obj_create(info_window);
    lv_obj_set_size(settings_obj, INFO_WINDOW_WIDTH, INFO_WINDOW_HEIGHT);
    lv_obj_align(settings_obj, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_bg_color(settings_obj, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(settings_obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(settings_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(settings_obj, 20, LV_PART_MAIN);
    
    // Title label
    lv_obj_t *title_label = lv_label_create(settings_obj);
    lv_label_set_text(title_label, "Refresh Speed");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_align(title_label, LV_ALIGN_TOP_LEFT, 0, 0);
    
    // Left label (Fast/Insensitive)
    lv_obj_t *left_label = lv_label_create(settings_obj);
    lv_label_set_text(left_label, "Fast/Insensitive");
    lv_obj_set_style_text_color(left_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(left_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(left_label, LV_ALIGN_TOP_LEFT, 0, 40);
    
    // Right label (Slow/Sensitive)
    lv_obj_t *right_label = lv_label_create(settings_obj);
    lv_label_set_text(right_label, "Slow/Sensitive");
    lv_obj_set_style_text_color(right_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_font(right_label, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_align(right_label, LV_ALIGN_TOP_RIGHT, 0, 40);
    
    // Load saved slider value from non-volatile storage
    // Convert old values (0-100 or 0-19) to new range (0-18) if needed
    uint8_t saved_raw = loadScanSpeed(9);  // Default to 9 (1020ms, middle of new range)
    uint8_t saved_slider_value = saved_raw;
    if (saved_raw > 18) {
        // Old value from previous version, convert to new range (0-18)
        if (saved_raw > 19) {
            // Very old value (0-100), convert to new range
            saved_slider_value = (saved_raw * 18) / 100;
        } else {
            // Previous version (0-19), convert to new range (0-18)
            saved_slider_value = (saved_raw * 18) / 19;
        }
    }
    
    // Create slider with discrete steps (0-18 = 19 positions, each 100ms from 120ms to 1920ms)
    // Range: 120ms to 1920ms in 100ms increments (matches ESP-IDF minimum of 120ms)
    lv_obj_t *slider = lv_slider_create(settings_obj);
    lv_obj_set_width(slider, INFO_WINDOW_WIDTH - 40);
    lv_obj_align(slider, LV_ALIGN_TOP_MID, 0, 60);
    lv_slider_set_range(slider, 0, 18);  // 19 discrete positions
    lv_slider_set_value(slider, saved_slider_value, LV_ANIM_OFF);
    
    // Style the slider
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x007acc), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x007acc), LV_PART_KNOB);
    
    // Add event handlers
    lv_obj_add_event_cb(slider, onRefreshSpeedChanged, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(slider, onRefreshSpeedChanged, LV_EVENT_RELEASED, NULL);
    
    // Initialize scan time based on slider's initial value
    // Formula: 120ms + (value * 100ms) = 120ms to 1920ms in 100ms steps
    extern uint16_t scan_time_per_channel_ms;
    scan_time_per_channel_ms = 120 + (saved_slider_value * 100);
    
    // Initially hidden (graph is default view)
    lv_obj_add_flag(settings_obj, LV_OBJ_FLAG_HIDDEN);
}



