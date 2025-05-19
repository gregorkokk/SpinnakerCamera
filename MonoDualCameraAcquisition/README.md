# Camera Control System

## Overview
This project provides a robust framework for controlling multiple FLIR cameras using the Spinnaker SDK. It allows for precise configuration of camera settings and synchronized image acquisition with region of interest (ROI) support.

## Features
- Multi-camera management and synchronization
- Custom camera settings through configuration files
- Dynamic ROI (Region of Interest) configuration
- Image acquisition with automatic file naming
- Graceful error handling and recovery
- User-controlled acquisition termination
- Automatic adaptation to camera capabilities

## File Structure
- `main.cpp` - Entry point that initializes the system and manages the main application flow
- `camera_manager.h/cpp` - Core camera control functionality including acquisition and configuration
- `camera_settings.h/cpp` - Settings parser and provider for camera configuration
- `Makefile` - Build system for compiling the application

## Requirements
- Spinnaker SDK (for FLIR cameras)
- C++11 or newer compiler
- Compatible FLIR cameras

## Installation
1. Install Spinnaker SDK according to the [FLIR documentation](https://www.flir.com/products/spinnaker-sdk/)
2. Clone this repository
3. Build using your preferred build system (CMake recommended)

## Usage
1. Configure camera settings in a text file (format `Key: Value`, e.g., `Exposure: 5000`)
2. Update the file paths in `main.cpp` for the settings file and image output directory
3. Compile and run the application
4. Images will be captured according to the ROI configuration
5. Press 'q' during acquisition to terminate the program gracefully

## Camera Settings
The following camera settings can be configured:
- `Exposure`: Camera exposure time in microseconds
- `Gain`: Camera gain value
- `Gamma`: Gamma correction value

## Image Acquisition Flow
1. System initializes and detects available cameras
2. Camera settings are loaded from the configuration file
3. Cameras are configured with appropriate settings (pixel format, exposure, gain, etc.)
4. The application cycles through predefined ROI configurations
5. Images are captured for each ROI and saved with descriptive filenames
6. User can terminate acquisition at any time by pressing 'q'

## Image Naming Convention
Images are saved with filenames following this pattern:
```
Serial_<camera-serial-number>_OffsetX_<offset-x>_Image_<index>.jpg
```

## ROI Configuration
The default ROI configurations are:
- ROI 1: `offset_x = 0, offset_y = 0, width = 1216, height = 352`
- ROI 2: `offset_x = 1216, offset_y = 0, width = 1216, height = 352`

These can be modified in the `camera_manager.h` file.

## Error Handling
The system includes robust error handling with:
- Multiple initialization retries
- Per-camera error recovery
- Detailed error reporting
- Graceful termination on critical errors

## Advanced Features
- Black level clamping for improved image quality
- Global shutter mode configuration
- Adaptive exposure timeout calculation
- Circular buffer for image storage (limits to 3 images per offset)

## Author
Gregor Kokk (2024)
