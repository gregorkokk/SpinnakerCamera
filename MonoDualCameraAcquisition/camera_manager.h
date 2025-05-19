// camera_manager.cpp Header File
// Author: Gregor Kokk
// Date: 06.01.2025

#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "camera_settings.h"

#include <iostream>
#include <string>
#include <vector>
#include <atomic>

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

class CAMERA_MANAGER
{
    private:
        // Pointer to the camera settings object
        const CAMERA_SETTINGS* camera_settings;

        int acquire_images(
            vector<CameraPtr>& cameras, 
            unsigned int number_of_cameras, 
            const vector<INodeMap*>& node_maps, 
            const vector<INodeMap*>& node_maps_tl_device, 
            atomic<bool>& global_running, 
            const string& folder_path
        );

        // Struct to hold ROI configuration values
        struct ROI_CONFIG_VALUES
        {
            int64_t offset_x;
            int64_t offset_y;
            int64_t width;
            int64_t height;
        };

        // Vector to hold ROI configuration values
        vector<ROI_CONFIG_VALUES> roi_config_values = 
        {
            {0, 0, 1216, 352},   // First ROI: offset_x = 0, offset_y = 0, width = 1216, height = 352
            {1216, 0, 1216, 352} // Second ROI: offset_x = 1216, offset_y = 0, width = 1216, height = 352
        };


    public:
        CAMERA_MANAGER(const CAMERA_SETTINGS* settings);    // Constructor
        ~CAMERA_MANAGER();   // Destructor

        // Function to get the camera serial number
        string get_camera_serial_number(INodeMap* node_map_tl_device, unsigned int camera_index);

        // Additional functions to make code more modular -> Database and Keyboard
        bool handle_keyboard_interrupt();   // Handles keyboard interrupts during image acquisition
        void set_non_blocking_input(bool enable); // Set Non Blocking Input

        // Additional functions to make code more modular -> Camera
        uint64_t calculate_exposure_timeout(INodeMap* node_map, unsigned int camera_index);
        bool is_camera_valid(const CameraPtr& camera, INodeMap* node_map, unsigned int camera_index);   // Checks if a camera_ptr and its node map are valid
        int set_acquisition_mode(INodeMap* node_map, unsigned int camera_index);   // Sets the acquisition mode to "Continuous"
        
        // Captures an image for a specific region based on OffsetX
        void capture_image(
            CameraPtr& camera,
            uint64_t timeout,
            const string& folder_path,
            const string& device_serial,
            unsigned int image_index,
            unsigned int camera_index,
            int64_t offset_x
        );

        void stop_camera_acquisition(vector<CameraPtr>& cameras); // Stops acquisition for the given camera
        void de_initialize_cameras(vector<CameraPtr>& cameras, vector<CameraPtr>& initialized_cameras, vector<INodeMap*>& node_maps, vector<INodeMap*>& node_maps_tl_device); // Cleanup Cameras
        
        int start_camera_acquisition(CameraPtr& camera, unsigned int camera_index);    // Starts acquisition for the given camera

        // Helper functions
        int print_device_info(INodeMap& node_map); // Print Device Information -> Do not need actually
        int keyboard_input(); // Function to get keyboard input
       
        // Configurations for the camera
        int config_pixel_format(const vector<INodeMap*>& node_maps); // Custom Pixel Format
        int config_roi(INodeMap* node_map, int64_t offset_x, int64_t offset_y, int64_t width, int64_t height, unsigned int camera_index); // Custom Region Of Interest
        int config_exposure(const vector<INodeMap*>& node_maps); // Custom Exposure Time
        int config_gamma(const vector<INodeMap*>& node_maps); // Custom Gamma
        int config_gain(const vector<INodeMap*>& node_maps); // Custom Gain
        int config_sensor_shutter_mode(const vector<INodeMap*>& node_maps); // Custom Sensor Shutter Mode
        int config_black_level_clamping_enable(const vector<INodeMap*>& node_maps); // Black Level Clamping
	    int reset_exposure(const vector<INodeMap*>& node_maps); // Reset Exposure Time

        // Runs the camera configuration and image acquisition
        int run_multiple_cameras(vector<CameraPtr>& cameras, CameraList& cam_list, unsigned int number_of_cameras, atomic<bool>& global_running, const string& folder_path);
};      
#endif // CAMERA_MANAGER_H
