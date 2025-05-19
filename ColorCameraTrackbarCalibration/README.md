# Color Camera Trackbar System

## Overview
This system provides an interactive framework for real-time adjustment and configuration of FLIR color cameras using the Spinnaker SDK and OpenCV. It allows users to fine-tune camera settings through interactive sliders while viewing the live camera feed, ideal for camera calibration and optimal setting determination.

## Features
- Real-time camera feed display with OpenCV
- Interactive sliders for adjusting camera parameters:
  - Exposure control
  - Gain adjustment
  - Gamma correction
  - Sharpening enhancement
  - Saturation adjustment
- BGR color format support for rich color imaging
- Custom ROI (Region of Interest) configuration
- One-click settings export to configuration file
- Non-blocking keyboard input for smooth operation
- Detailed error handling and parameter range validation

## File Structure
- `color_main_trackbar.cpp` - Implementation of the interactive color camera configuration system
- `main.h` - Header file defining the CAMERA_CONFIG class and its methods
- `Makefile` - Build system for compiling the application

## Requirements
- Spinnaker SDK (for FLIR cameras)
- OpenCV library (for display and UI elements)
- C++11 or newer compiler
- Compatible FLIR color camera (tested with BFS-U3-50S5C-C Blackfly S)
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
   ./color_camera_trackbar
   ```
2. Use the sliders to adjust camera settings in real-time:
   - **Exposure**: Controls the amount of light captured (33.0 μs to 500,000.0 μs)
   - **Gain**: Amplifies the signal (0.0 to 48.0 dB)
   - **Sharpening**: Enhances edge detail (-1.0 to 8.0)
   - **Gamma**: Adjusts image brightness curve (0.25 to 4.0)
   - **Saturation**: Controls color intensity (0.0 to 1.0)
3. Press 'q' to save the current settings to a configuration file and exit

## Camera Settings
The system provides sliders with the following ranges:

| Setting      | Slider Range | Actual Value Range    | Description                       |
|--------------|--------------|----------------------|-----------------------------------|
| Exposure     | 0 - 10000    | 33.0 μs - 500,000.0 μs | Amount of light captured         |
| Gain         | 0 - 48       | 0.0 - 48.0 dB        | Signal amplification             |
| Sharpening   | 0 - 9        | -1.0 - 8.0           | Edge enhancement                 |
| Gamma        | 0 - 100      | 0.25 - 4.0           | Brightness curve adjustment      |
| Saturation   | 0 - 20       | 0.0 - 1.0            | Color intensity                  |

## Configuration File
When you press 'q' to exit, the current settings are saved to a configuration file:
```
/home/aire/xavier/spinnaker/src/database_color.txt
```

The file format is:
```
Exposure: 5000.0 [μs]
Gain: 5.0 [dB]
Sharpening: 1.5
Gamma: 0.8
Saturation: 0.7
```

This file can be used with the Color Camera Infinity Capture System for consistent settings.

## Display Window
The system creates a window with the following properties:
- Title: "Display window"
- Size: 408x408 pixels (customizable via constants)
- Controls: Five trackbars for real-time parameter adjustment
- Display: Live feed from the camera with settings applied

## System Flow
1. System initializes and detects available cameras
2. Camera is initialized and configured with default settings
3. OpenCV window is created with control sliders
4. Camera begins continuous image acquisition in BGR8 format
5. Images are displayed in real-time with current settings applied
6. User adjusts settings via sliders, seeing immediate results
7. When the user presses 'q', settings are saved to the database file
8. Camera is reset to automatic exposure and deinitialized

## ROI Configuration
The default ROI configuration is set to create a square image:
- Width: Set to camera_screen_height (408 pixels by default)
- Height: Set to camera_screen_width (408 pixels by default)
- X Offset: 0
- Y Offset: 0

## Error Handling
The system includes robust error handling:
- Parameter range validation for all camera settings
- Auto-adjustment of out-of-range values to min/max limits
- Detailed error reporting for all camera operations
- Graceful termination on critical errors

## Interactivity
- Slider adjustments are immediately applied to the camera
- Visual feedback is provided in real-time
- Keyboard monitoring allows for smooth termination
- Settings are automatically saved upon exit

## Author
Gregor Kokk (2024)
