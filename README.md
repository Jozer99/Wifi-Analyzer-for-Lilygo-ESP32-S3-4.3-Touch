# WiFi Analyzer for Lilygo ESP32-S3 4.3 Touch

A WiFi network analyzer application for the ESP32-S3 development board with a 4.3" touch display. This project provides real-time visualization of WiFi networks using LVGL graphics library.

## Features

- Real-time WiFi network scanning and visualization
- Interactive touch-based UI using LVGL
- Signal strength (RSSI) graphing
- Network information display
- Multi-screen navigation

## Hardware Requirements

- ESP32-S3 development board (ESP32-S3-DevKitC-1)
- 4.3" touch display panel
- Compatible with Lilygo ESP32-S3 4.3" Touch display

## Software Requirements

- PlatformIO IDE
- Arduino framework
- LVGL 8.3.8
- ESP32_Display_Panel library
- ESP32_IO_Expander library

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/Jozer99/Wifi-Analyzer-for-Lilygo-ESP32-S3-4.3-Touch.git
   cd Wifi-Analyzer-for-Lilygo-ESP32-S3-4.3-Touch
   ```

2. Open the project in PlatformIO

3. Install dependencies (PlatformIO will handle this automatically)

4. Build and upload to your ESP32-S3 board

## Configuration

Edit `src/config.h` to customize display settings, graph dimensions, and other parameters.

## Project Structure

```
├── src/              # Source code
│   ├── main.cpp      # Main entry point
│   ├── wifi_scanner.cpp  # WiFi scanning logic
│   ├── wifi_data.cpp     # Data visualization
│   ├── ui_views.cpp      # UI view definitions
│   └── ui_handlers.cpp   # UI event handlers
├── lib/              # Library dependencies
├── platformio.ini    # PlatformIO configuration
└── README.md         # This file
```

## License

[Add your license here]

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

