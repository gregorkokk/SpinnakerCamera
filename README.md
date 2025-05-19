# Spinnaker Camera Tools

A collection of C++ utilities for controlling FLIR cameras using the Spinnaker SDK.

## Overview

This repository contains a suite of C++ tools for controlling FLIR cameras (primarily Blackfly S models) using the Spinnaker SDK. Each tool is designed for a specific use case:

- **ColorCameraInfinityCapture**: Continuous image capture from color cameras with enhanced image settings
- **MonoCameraInfinityCapture**: Continuous image capture from monochrome cameras with specialized mono features
- **ColorCameraTrackbar**: Interactive calibration tool for color cameras with real-time parameter adjustment
- **MonoCameraTrackbar**: Interactive calibration tool for monochrome cameras with real-time parameter adjustment
- **MonoDualCameraAcquisition**: Advanced system for synchronized image acquisition from multiple monochrome cameras with ROI support

## Requirements

- Spinnaker SDK
- C++11 or newer compiler
- Compatible FLIR camera (tested with BFS-U3-50S5C-C and BFS-U3-50S5M-C Blackfly S)
- Linux environment
- OpenCV (for trackbar tools only)

## Quick Start

1. Navigate to the desired tool directory
2. Compile using the provided Makefile:
   ```
   make
   ```
3. Run the application:
   ```
   ./[executable_name]
   ```

## Workflow

The typical workflow is:

1. Use the appropriate Trackbar tool to visually calibrate your camera settings
2. Settings are automatically saved to a configuration file
3. Use the matching Infinity Capture tool which will read these settings for continuous image acquisition

For dual camera setups:
1. Configure settings in the mono.txt configuration file
2. Run the MonoDualCameraAcquisition tool to capture synchronized images with custom ROIs from multiple cameras

## Key Features

- **Programming Language**: All tools written in C++11
- **Image Processing**: BGR8 color and Mono8 monochrome formats
- **Camera Configuration**: Exposure, gain, gamma, and specialized settings for each camera type
- **Synchronized Capture**: Coordinated acquisition from multiple cameras
- **ROI Support**: Custom regions of interest for focused imaging
- **Black Level Clamping**: Enhanced contrast for monochrome cameras
- **Global Shutter Mode**: Distortion-free capture of moving objects
- **Interactive Calibration**: Real-time visual parameter adjustment

## Author

Gregor Kokk (2024)