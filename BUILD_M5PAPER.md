# M5Paper Build Instructions

## Prerequisites

1. Install PlatformIO Core:
   ```bash
   pip install platformio
   ```

2. Install ESP-IDF dependencies (PlatformIO will handle most of this automatically)

## Building

```bash
# Clean build
pio run -e m5paper -t clean

# Build firmware
pio run -e m5paper

# Upload to device
pio run -e m5paper -t upload

# Monitor serial output
pio device monitor
```

## Configuration

The build uses these key configurations:
- **Board**: m5stack-fire (ESP32)
- **Framework**: ESP-IDF
- **Target**: m5paper environment in platformio.ini
- **Library**: M5EPD v0.1.5+

## Known Issues / TODO

1. **M5EPD Library Integration**: The M5EPD library needs to be properly integrated with the existing EPub reader codebase
   - Screen driver may need adjustments for proper grayscale rendering
   - Touch input needs to be mapped to book navigation controls
   
2. **SD Card Path**: M5EPD uses different SD card mount point - may need adjustments in file paths

3. **Memory Management**: Verify PSRAM configuration for ESP32 Fire

4. **Testing Required**:
   - Screen refresh rates (full vs partial updates)
   - Touch screen calibration and responsiveness
   - Font rendering quality
   - Image dithering
   - Battery management
   - RTC functionality

## Troubleshooting

If build fails:
1. Check that ESP-IDF toolchain is properly installed
2. Verify M5EPD library is accessible
3. Check FreeType library compilation (in lib_freetype/)
4. Review platformio.ini for correct board and framework settings
