/*
 * LVGL porting layer for ESP32-S3
 */

#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include <lvgl.h>
#include <ESP_Panel_Library.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Global panel and mutex (extern declarations)
extern ESP_Panel *panel;
extern SemaphoreHandle_t lvgl_mux;

// Functions
void lvgl_port_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void lvgl_port_tp_read(lv_indev_drv_t *indev, lv_indev_data_t *data);
void lvgl_port_lock(int timeout_ms);
void lvgl_port_unlock(void);
void lvgl_port_task(void *arg);
bool notify_lvgl_flush_ready(void *user_ctx);
void lvgl_port_init(void);

#endif // LVGL_PORT_H



