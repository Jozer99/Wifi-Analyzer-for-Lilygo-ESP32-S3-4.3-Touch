/*
 * UI event handlers implementation
 */

#include "ui_handlers.h"
#include "ui_views.h"
#include "wifi_scanner.h"
#include "preferences_storage.h"
#include "lvgl_port.h"
#include "wifi_data.h"

// External state (declared in main.cpp)
extern bool scanning_paused;
extern bool persistence_enabled;

// View switching functions
void switchToGraphView(lv_event_t *e) {
    lvgl_port_lock(-1);
    
    // Hide all views
    if (table_obj) lv_obj_add_flag(table_obj, LV_OBJ_FLAG_HIDDEN);
    if (table_header) lv_obj_add_flag(table_header, LV_OBJ_FLAG_HIDDEN);
    if (settings_obj) lv_obj_add_flag(settings_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Show graph view
    if (graph_obj) lv_obj_clear_flag(graph_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Update button states
    if (graph_btn) {
        lv_obj_add_state(graph_btn, LV_STATE_CHECKED);
    }
    if (table_btn) {
        lv_obj_clear_state(table_btn, LV_STATE_CHECKED);
    }
    if (settings_btn) {
        lv_obj_clear_state(settings_btn, LV_STATE_CHECKED);
    }
    
    lvgl_port_unlock();
}

void switchToTableView(lv_event_t *e) {
    lvgl_port_lock(-1);
    
    // Hide all views
    if (graph_obj) lv_obj_add_flag(graph_obj, LV_OBJ_FLAG_HIDDEN);
    if (settings_obj) lv_obj_add_flag(settings_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Show table view and header
    if (table_obj) lv_obj_clear_flag(table_obj, LV_OBJ_FLAG_HIDDEN);
    if (table_header) lv_obj_clear_flag(table_header, LV_OBJ_FLAG_HIDDEN);
    
    // Update button states
    if (table_btn) {
        lv_obj_add_state(table_btn, LV_STATE_CHECKED);
    }
    if (graph_btn) {
        lv_obj_clear_state(graph_btn, LV_STATE_CHECKED);
    }
    if (settings_btn) {
        lv_obj_clear_state(settings_btn, LV_STATE_CHECKED);
    }
    
    lvgl_port_unlock();
}

void switchToSettingsView(lv_event_t *e) {
    lvgl_port_lock(-1);
    
    // Hide all views
    if (graph_obj) lv_obj_add_flag(graph_obj, LV_OBJ_FLAG_HIDDEN);
    if (table_obj) lv_obj_add_flag(table_obj, LV_OBJ_FLAG_HIDDEN);
    if (table_header) lv_obj_add_flag(table_header, LV_OBJ_FLAG_HIDDEN);
    
    // Show settings view
    if (settings_obj) lv_obj_clear_flag(settings_obj, LV_OBJ_FLAG_HIDDEN);
    
    // Update button states
    if (settings_btn) {
        lv_obj_add_state(settings_btn, LV_STATE_CHECKED);
    }
    if (graph_btn) {
        lv_obj_clear_state(graph_btn, LV_STATE_CHECKED);
    }
    if (table_btn) {
        lv_obj_clear_state(table_btn, LV_STATE_CHECKED);
    }
    
    lvgl_port_unlock();
}

void togglePause(lv_event_t *e) {
    scanning_paused = !scanning_paused;
    
    lvgl_port_lock(-1);
    
    if (pause_btn) {
        if (scanning_paused) {
            lv_obj_add_state(pause_btn, LV_STATE_CHECKED);
            lv_obj_t *label = lv_obj_get_child(pause_btn, 0);
            if (label) lv_label_set_text(label, "Resume");
        } else {
            lv_obj_clear_state(pause_btn, LV_STATE_CHECKED);
            lv_obj_t *label = lv_obj_get_child(pause_btn, 0);
            if (label) lv_label_set_text(label, "Pause");
        }
    }
    
    lvgl_port_unlock();
}

void togglePersistence(lv_event_t *e) {
    persistence_enabled = !persistence_enabled;
    
    lvgl_port_lock(-1);
    
    if (persistence_btn) {
        if (persistence_enabled) {
            lv_obj_add_state(persistence_btn, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(persistence_btn, LV_STATE_CHECKED);
            // Clear persistent network list when turning off
            clearPersistentNetworks();
        }
    }
    
    lvgl_port_unlock();
}

void onRefreshSpeedChanged(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    
    // Ensure value is within valid range (0-18) for discrete steps
    if (value < 0) value = 0;
    if (value > 18) value = 18;
    
    // Map slider value (0-18) to scan time (120ms to 1920ms in 100ms steps)
    // Formula: 120ms + (value * 100ms)
    // This matches ESP-IDF's minimum of 120ms
    scan_time_per_channel_ms = 120 + (value * 100);
    
    // Save to non-volatile storage
    saveScanSpeed((uint8_t)value);
}

