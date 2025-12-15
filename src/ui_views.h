/*
 * UI view creation functions
 */

#ifndef UI_VIEWS_H
#define UI_VIEWS_H

#include <lvgl.h>

// Global UI objects (extern declarations)
extern lv_obj_t *graph_obj;
extern lv_obj_t *vertical_axis_label;
extern lv_obj_t *table_obj;
extern lv_obj_t *table_header;
extern lv_obj_t *settings_obj;
extern lv_obj_t *info_window;
extern lv_obj_t *menu_bar;
extern lv_obj_t *pause_btn;
extern lv_obj_t *persistence_btn;
extern lv_obj_t *graph_btn;
extern lv_obj_t *table_btn;
extern lv_obj_t *settings_btn;

// Functions
void createMenuBar(lv_obj_t *parent);
void createTableView();
void createSettingsView();

#endif // UI_VIEWS_H



