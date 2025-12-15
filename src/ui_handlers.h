/*
 * UI event handlers
 */

#ifndef UI_HANDLERS_H
#define UI_HANDLERS_H

#include <lvgl.h>

// Functions
void switchToGraphView(lv_event_t *e);
void switchToTableView(lv_event_t *e);
void switchToSettingsView(lv_event_t *e);
void togglePause(lv_event_t *e);
void togglePersistence(lv_event_t *e);
void onRefreshSpeedChanged(lv_event_t *e);

#endif // UI_HANDLERS_H



