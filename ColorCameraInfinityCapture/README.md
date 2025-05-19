# Color Camera Infinity Capture System

## Overview
This system provides a comprehensive framework for continuous image capture from FLIR color cameras using the Spinnaker SDK. It's designed to configure the camera with optimal settings and capture images continuously until the user chooses to stop the process.

## Features
- Continuous image capture until user termination
- Comprehensive camera configuration options:
  - Exposure control
  - Gain adjustment
  - Gamma correction
  - Sharpening enhancement
  - Saturation adjustment
- BGR color format support for rich color imaging
- Custom ROI (Region of Interest) configuration
- Time-stamped image naming for sequence tracking
- Non-blocking keyboard input for smooth operation termination
- Adaptive frame rate control with timing information
- Detailed error handling and reporting

## File Structure
- `main_color_infinity_images.cpp` - Implementation of the color camera capture system
- `main.h` - Header file defining the CAMERA_CONFIG class and its methods
- `Makefile` - Build system for compiling the application

## Requirements
- Spinnaker SDK (for FLIR cameras)
- C++11 or newer compiler
- Compatible FLIR camera (tested with BFS-U3-50S5C-C Blackfly S)
- Linux environment (uses termios.h for keyboard input)

## Installation
1. Install Spinnaker SDK according to the [FLIR documentation](https://www.flir.com/products/spinnaker-sdk/)
2. Clone this repository
3. Compile the application using `make`

## Usage
1. Create a configuration file (`database_color.txt`) with the following format:
   ```
   Exposure: 5000
   Gain: 5.0
   Sharpening: 1.5
   Gamma: 0.8
   Saturation: 0.7
   ```

2. Run the application:
   ```
   ./color_camera_capture
   ```

3. Press 'q' at any time to gracefully terminate the image acquisition process

## Camera Settings
The system supports the following camera configurations:

| Setting      | Description                            | Typical Range      |
|--------------|----------------------------------------|-------------------|
| Exposure     | Camera exposure time in microseconds   | 33.0 μs - 30.0 s  |
| Gain         | Camera gain value in dB                | 0.0 - 47.99 dB    |
| Gamma        | Gamma correction value                 | 0.1 - 4.0         |
| Sharpening   | Image sharpening enhancement           | -1.0 - 8.0        |
| Saturation   | Color saturation adjustment            | 0.0 - 1.0         |

## Image Acquisition Flow
1. System initializes and detects available cameras
2. Camera settings are loaded from the configuration file
3. The camera is configured with appropriate settings:
   - Pixel format (BGR8)
   - ROI (1424 x 408 pixels by default)
   - Exposure, gain, gamma, sharpening, and saturation
4. The camera begins continuous image acquisition
5. Images are saved with timestamps in the specified directory
6. The loop continues until the user presses 'q' to terminate
7. Camera is reset to automatic exposure and deinitialized

## Image Naming Convention
Images are saved with filenames following this pattern:
```
image_<count>_<minutes>:<seconds>.jpg
```
Where:
- `count` is the sequential image number
- `minutes` and `seconds` represent the elapsed time since the start of acquisition

## ROI Configuration
The default ROI configuration is:
- Width: 1424 pixels
- Height: 408 pixels

This can be modified in the `config_roi` function call in `run_single_camera` method.

## Error Handling
The system includes robust error handling:
- Parameter range validation for all camera settings
- Detailed error reporting for all camera operations
- Graceful termination on critical errors

## Author
Gregor Kokk (2024)