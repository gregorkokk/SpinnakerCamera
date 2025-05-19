// color_capture_image_main.cpp header file
// Author: Gregor Kokk
// Date: 2024

#ifndef MAIN_H
#define MAIN_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

class CAMERA_CONFIG
{
    private:

        struct camera_settings  // To hold settings for the camera
        {
            double exposure;
            double gain;
            double sharpening;
            double gamma;
            double saturation;
        };

        camera_settings settings; // Struct

        double extract_value_from_line(const string &line); // Extract Value From Line

        static int reset_exposure(INodeMap& node_map); // Reset Exposure Time
        static int acquire_images(CameraPtr pointer_cam, INodeMap& node_map, INodeMap& node_map_tl_device); // Acquire And Save Images From The Camera

    public:

        vector<string> load_from_file(const string &filename); // Load From File

        static int print_device_info(INodeMap& node_map); // Print Device Information

        int config_pixel_format(INodeMap& node_map); // Custom Pixel Format
        int config_roi(INodeMap& node_map, int64_t width_value, int64_t height_value); // Custom Region Of Interest
        int run_single_camera(CameraPtr pointer_cam);   // Main Function For Camera Configuration
        int keyboard_input(); // Keyboard Input

        int config_exposure(INodeMap& node_map); // Custom Exposure Time
        int config_sharpening(INodeMap& node_map); // Custom Sharpering
        int config_gamma(INodeMap& node_map); // Custom Gamma
        int config_gain(INodeMap& node_map); // Custom Gain
        int config_saturation(INodeMap& node_map); // Custom Saturation
        
        void set_non_blocking_input(bool enable); // Set Non-Blocking Input
        void get_values(const vector<string> &file_content); // Get Values
};

#endif // MAIN_H