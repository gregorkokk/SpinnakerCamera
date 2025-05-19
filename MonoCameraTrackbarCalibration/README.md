# Mono Camera Trackbar System

## Overview
This system provides an interactive framework for real-time adjustment and configuration of FLIR monochrome cameras using the Spinnaker SDK and OpenCV. It allows users to fine-tune camera settings through interactive sliders while viewing the live camera feed, ideal for camera calibration and determining optimal settings for monochrome imaging.

## Features
- Real-time camera feed display with OpenCV
- Interactive sliders for adjusting camera parameters:
  - Exposure control
  - Gain adjustment
  - Gamma correction
- Mono8 format support for efficient monochrome imaging
- Custom ROI (Region of Interest) configuration
- One-click settings export to configuration file
- Specialized monochrome enhancements:
  - Global shutter mode
  - Black level clamping
- Non-blocking keyboard input for smooth operation
- Detailed error handling and parameter range validation

## File Structure
- `mono_main_trackbar.cpp` - Implementation of the interactive monochrome camera configuration system
- `main.h` - Header file defining the CAMERA_CONFIG class and its methods
- `Makefile` - Build system for compiling the application

## Requirements
- Spinnaker SDK (for FLIR cameras)
- OpenCV library (for display and UI elements)
- C++11 or newer compiler
- Compatible FLIR monochrome camera (tested with BFS-U3-50S5M-C Blackfly S)
- Linux environment (uses termios.h for keyboard input)

## Installation
1. Install Spinnaker SDK according to the [FLIR documentation](https://www.flir.com/products/spinnaker-sdk/)
2. Install OpenCV:
   ```
   sudo apt-get install libopencv-dev
   ```
3. Clone this repository
4. Compile the application using `make`

## Usage
1. Run the application:
   ```
   ./mono_camera_trackbar
   ```
2. Use the sliders to adjust camera settings in real-time:
   - **Exposure**: Controls the amount of light captured (33.0 μs to 150,000.0 μs)
   - **Gain**: Amplifies the signal (0.0 to 48.0 dB)
   - **Gamma**: Adjusts image brightness curve (0.25 to 4.0)
3. Press 'q' to save the current settings to a configuration file and exit

## Camera Settings
The system provides sliders with the following ranges:

| Setting      | Slider Range | Actual Value Range    | Description                       |
|--------------|--------------|----------------------|-----------------------------------|
| Exposure     | 0 - 10000    | 33.0 μs - 150,000.0 μs | Amount of light captured         |
| Gain         | 0 - 48       | 0.0 - 48.0 dB        | Signal amplification             |
| Gamma        | 0 - 100      | 0.25 - 4.0           | Brightness curve adjustment      |

## Special Monochrome Features
The system includes specific features optimized for monochrome imaging:

### Global Shutter Mode
Ensures all pixels are exposed simultaneously, which is ideal for capturing moving objects without distortion. This is particularly important for applications like machine vision where image consistency is critical.

### Black Level Clamping
Enhances image contrast and dynamic range by accurately setting the black level reference point. This improves the overall image quality, especially in low-light conditions or scenes with high dynamic range.

### High-Quality Linear Processing
Applies SPINNAKER_COLOR_PROCESSING_ALGORITHM_HQ_LINEAR for optimal monochrome image processing, ensuring the best possible image quality with accurate grayscale representation.

## Configuration File
When you press 'q' to exit, the current settings are saved to a configuration file:
```
/home/vikan/xavier/spinnaker/src/database_mono.txt
```

The file format is:
```
Exposure: 5000.0 [μs]
Gain: 5.0 [dB]
Gamma: 0.8
```

This file can be used with the Mono Camera Infinity Capture System for consistent settings.

## Display Window
The system creates a window with the following properties:
- Title: "Display window"
- Size: 1424x375 pixels (customizable via constants)
- Controls: Three trackbars for real-time parameter adjustment
- Display: Live feed from the camera with settings applied

## ROI Configuration
The default ROI configuration is:
- Width: 1424 pixels (customizable via camera_screen_width)
- Height: 375 pixels (customizable via camera_screen_height)
- X Offset: 0
- Y Offset: 0

## System Flow
1. System initializes and detects available cameras
2. Camera is initialized and configured with default settings
3. OpenCV window is created with control sliders
4. Camera begins continuous image acquisition in Mono8 format
5. Images are displayed in real-time with current settings applied
6. User adjusts settings via sliders, seeing immediate results
7. When the user presses 'q', settings are saved to the database file
8. Camera is reset to automatic exposure and deinitialized

## Error Handling
The system includes robust error handling:
- Parameter range validation for all camera settings
- Auto-adjustment of out-of-range values to min/max limits
- Detailed error reporting for all camera operations
- Graceful termination on critical errors

## Multi-Camera Support
The system is designed to work with multiple cameras simultaneously:
- Detects all connected monochrome cameras
- Applies the same configuration to each camera
- Processes each camera sequentially

## Author
Gregor Kokk (2024)
