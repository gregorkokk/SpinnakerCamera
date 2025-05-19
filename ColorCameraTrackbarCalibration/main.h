// live_color_cam_image_main.cpp header file
// Author: Gregor Kokk
// Date: 2024

#ifndef MAIN_H
#define MAIN_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include <iostream>
#include <sstream>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;

class CAMERA_CONFIG
{
    private:
        static int reset_exposure(INodeMap& node_map); // Reset Exposure Time
        static int acquire_and_display_images(CameraPtr pointer_cam, INodeMap& node_map, INodeMap& node_map_tl_device); // Acquire And Save Images From The Camera

    public:	
        int config_exposure(INodeMap& node_map, double current_exposure_value); // Custom Exposure Time
        int config_gain(INodeMap& node_map, double current_gain_value); // Custom Gain
        int config_sharpening(INodeMap& node_map, double current_gain_value); // Custom Sharpening
        int config_gamma(INodeMap& node_map, double current_gamma_value); // Custom Gamma
        int config_saturation(INodeMap& node_map, double current_saturation_value); // Custom Saturation

        int config_pixel_format(INodeMap& node_map); // Custom Pixel Format
        int config_roi(INodeMap& node_map, int64_t width_value, int64_t height_value, int64_t x_offset_value, int64_t y_offset_value); // Custom Region Of Interest
        int run_single_camera(CameraPtr pointer_cam);   // Main Function For Camera Configuration
        int keyboard_input(); // Keyboard Input
        
        void set_non_blocking_input(bool enable); // Set Non-Blocking Input
        void setup_trackbar_and_window(CameraPtr pointer_cam); // Setup Trackbar And Window For Displaying Images
};

// Comment explaining why config_exposure is static
// config_exposure does not rely on any instance-specific data 
// so I made it static so that we do not need an object to call it.


#endif // MAIN_H
