/*
 * Configuration constants and defines for WiFi Radar System
 */

#ifndef CONFIG_H
#define CONFIG_H

// Extend IO Pin define
#define TP_RST 1
#define LCD_BL 2
#define LCD_RST 3
#define SD_CS 4
#define USB_SEL 5

// I2C Pin define
#define I2C_MASTER_NUM 0
#define I2C_MASTER_SDA_IO 8
#define I2C_MASTER_SCL_IO 9

// WiFi scan interval in milliseconds (1 second)
#define SCAN_INTERVAL_MS 1000

// LVGL porting configurations
#define LVGL_TICK_PERIOD_MS     (2)
#define LVGL_TASK_MAX_DELAY_MS  (500)
#define LVGL_TASK_MIN_DELAY_MS  (1)
#define LVGL_TASK_STACK_SIZE    (4 * 1024)
#define LVGL_TASK_PRIORITY      (2)
#define LVGL_BUF_SIZE           (ESP_PANEL_LCD_H_RES * 20)

// UI Layout dimensions
#define INFO_WINDOW_WIDTH 640     // Left region: information window
#define INFO_WINDOW_HEIGHT 480
#define MENU_BAR_WIDTH 160         // Right region: menu bar
#define MENU_BAR_HEIGHT 480

// Graph dimensions (within info window)
#define GRAPH_CANVAS_WIDTH INFO_WINDOW_WIDTH
#define GRAPH_LEFT_MARGIN 60      // Space for RSSI axis labels and vertical axis title
#define GRAPH_RIGHT_MARGIN 10
#define GRAPH_TOP_OFFSET 10       // Offset from top of canvas for graph content (room for labels)
#define GRAPH_BOTTOM_MARGIN 40    // Space for channel labels and axis title
#define GRAPH_WIDTH (GRAPH_CANVAS_WIDTH - GRAPH_LEFT_MARGIN - GRAPH_RIGHT_MARGIN)
#define GRAPH_HEIGHT (INFO_WINDOW_HEIGHT - GRAPH_TOP_OFFSET - GRAPH_BOTTOM_MARGIN)
#define RSSI_MIN -100
#define RSSI_MAX -30
#define CHANNEL_MIN -1
#define CHANNEL_MAX 15

#endif // CONFIG_H



