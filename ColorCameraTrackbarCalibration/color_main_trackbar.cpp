// BFS-U3-50S5C(Blackfly S) test trackbar for exposure times
// Author: Gregor Kokk
// Date: 2024

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include <iostream>
#include <sstream>
#include <chrono>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <fstream>
#include <iomanip> // For std::fixed

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

#include "main.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;
using namespace cv;

// width and height
const int camera_screen_width = 408;
const int camera_screen_height = 408;

// Variables for exposure
const int exposure_slider_max_value = 10000; // Maximum value for the trackbar
int exposure_value_slider = 200;  // Global variable for trackbar position (0 to 10000)
const double min_exposure = 33.0; // Minimum exposure time in microseconds
const double max_exposure = 500000.0; // Maximum exposure time in microseconds
double current_exposure_value; // Initial exposure time in microseconds

// Variables for gain
const int gain_slider_max_value = 48; // Maximum value for the trackbar
int gain_value_slider = 12;  // Global variable for gain value
const double min_gain = 0.0; // Minimum gain value
const double max_gain = 48.0; // Maximum gain value
double current_gain_value; // Initial gain value

// Variables for sharpening
const int sharpening_slider_max_value = 9; // Maximum value for the trackbar (since the range is from -1 to 8)
int sharpening_value_slider = 4;  // Global variable for trackbar position (0 to 9)
double current_sharpening_value; // Initial sharpening value

// Variables for gamma
const int gamma_slider_max_value = 100; // Maximum value for the trackbar
int gamma_value_slider = 2;  // Global variable for trackbar position (0 to 100)
const double min_gamma = 0.25; // Minimum gamma value
const double max_gamma = 4.0; // Maximum gamma value
double current_gamma_value; // Initial gamma value

// Variables for shaprening
const int saturation_slider_max_value = 20; // Trackbar value range from 0 to 20 (0.0 to 1.0 in steps of 0.05)
int saturation_value_slider = 5; // Initial slider value
double current_saturation_value; // Initial saturation value

// Camera settings variables - stores the value after it has been applied to the camera
double exposure_value;
double gain_value;
double sharpening_value;
double gamma_value;
double saturation_value;

// Callback function for saturation trackbar
static void trackbar_callback_saturation(int, void* camera_pointer)
{
    CAMERA_CONFIG camera_config;    // Create an instance of the CAMERA_CONFIG class

    CameraPtr* pointer_cam = reinterpret_cast<CameraPtr*>(camera_pointer);

    // Calculate the saturation value based on the trackbar value
    current_saturation_value = (static_cast<double>(saturation_value_slider) / 20.0);

    // Update the camera saturation value
    camera_config.config_saturation((*pointer_cam)->GetNodeMap(), current_saturation_value);
    saturation_value = current_saturation_value;
    cout << "Saturation value: " << saturation_value << endl;
}

// Callback function for gain trackbar
static void trackbar_callback_gamma(int, void* camera_pointer)
{
    CAMERA_CONFIG camera_config;    // Create an instance of the CAMERA_CONFIG class

    CameraPtr* pointer_cam = reinterpret_cast<CameraPtr*>(camera_pointer);

    // Calculate the gamma value based on the trackbar value
    current_gamma_value = min_gamma + ((static_cast<double>(gamma_value_slider) / gamma_slider_max_value) * (max_gamma - min_gamma));

    // Update the camera gamma value
    camera_config.config_gamma((*pointer_cam)->GetNodeMap(), current_gamma_value);
    gamma_value = current_gamma_value;
    cout << "Gamma value: " << gamma_value << endl;
}

// Callback function for gain trackbar
static void trackbar_callback_gain(int, void* camera_pointer)
{
    CAMERA_CONFIG camera_config;    // Create an instance of the CAMERA_CONFIG class

    CameraPtr* pointer_cam = reinterpret_cast<CameraPtr*>(camera_pointer);

    // Calculate the gain value based on the trackbar value and min/max limits
    current_gain_value = min_gain + ((static_cast<double>(gain_value_slider) / gain_slider_max_value) * (max_gain - min_gain));

    // Update the camera gain value
    camera_config.config_gain((*pointer_cam)->GetNodeMap(), current_gain_value);
    gain_value = current_gain_value;
    cout << "Gain value: " << gain_value << " dB" << endl;
}

// Callback function for sharpening trackbar
static void trackbar_callback_sharpening(int, void* camera_pointer)
{
    CAMERA_CONFIG camera_config;    // Create an instance of the CAMERA_CONFIG class

    CameraPtr* pointer_cam = reinterpret_cast<CameraPtr*>(camera_pointer);

    // Calculate the sharpening value based on the trackbar value
    current_sharpening_value = sharpening_value_slider - 1; // Subtract 1 for actual sharpening range
    

    // Update the camera sharpening value
    camera_config.config_sharpening((*pointer_cam)->GetNodeMap(), current_sharpening_value);
    sharpening_value = current_sharpening_value;
    cout << "Sharpening value: " << sharpening_value << endl;
}

// Callback function for exposure trackbar
static void trackbar_callback_exposure(int, void* camera_pointer)
{
    CAMERA_CONFIG camera_config;    // Create an instance of the CAMERA_CONFIG class

    CameraPtr* pointer_cam = reinterpret_cast<CameraPtr*>(camera_pointer);

    // Calculate the exposure time based on the trackbar value and min/max limits
    current_exposure_value = min_exposure + (static_cast<double>(exposure_value_slider) / exposure_slider_max_value) * (max_exposure - min_exposure);

    // Update the camera exposure value
    camera_config.config_exposure((*pointer_cam)->GetNodeMap(), current_exposure_value);
    exposure_value = current_exposure_value;
    cout << "Exposure value: " << exposure_value << " μs" << endl;
}

// This function saves the current camera settings to a database
void save_data_to_database()
{
    try
    {
        // Save exposure value to a file
        ofstream database_file("/home/aire/xavier/spinnaker/src/database_color.txt");
        if(database_file.is_open())
        {
            database_file << std::fixed << std::setprecision(1) << "Exposure: " << exposure_value << " [μs] \n"; // Save the current exposure value to a file
            database_file << std::fixed << std::setprecision(1) << "Gain: " << gain_value << " [dB] \n"; // Save the current gain value to a file
            database_file << std::fixed << std::setprecision(1) << "Sharpening: " << sharpening_value << " \n"; // Save the current sharpening value to a file
            database_file << std::fixed << std::setprecision(1) << "Gamma: " << gamma_value << " \n"; // Save the current gamma value to a file
            database_file << std::fixed << std::setprecision(1) << "Saturation: " << saturation_value << " \n"; // Save the current saturation value to a file

            if (database_file.good())
            {
                cout << "Data saved to database" << endl;
            }
            else
            {
                cout << "Error saving data to database" << endl;
            }
            database_file.close(); // Close the file
        }
        else
        {
            cout << "Unable to open file" << endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

// This function controls saturation
int CAMERA_CONFIG::config_saturation(INodeMap& node_map, double current_saturation_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING SATURATION ***" << endl << endl;

    try
    {
        CBooleanPtr ptr_saturation_enable = node_map.GetNode("SaturationEnable");    // Turn on saturation
        if (IsReadable(ptr_saturation_enable) && IsWritable(ptr_saturation_enable))
        {
            ptr_saturation_enable->SetValue(true);
            cout << "Saturation enabled..." << endl;

            CFloatPtr ptr_saturation = node_map.GetNode("Saturation");
            if (!IsReadable(ptr_saturation) || !IsWritable(ptr_saturation))
            {
                cout << "Custom saturation format not readable or writable" << endl;
                
                return -1;
            }

            if (current_saturation_value > ptr_saturation->GetMax())
            {
                current_saturation_value = ptr_saturation->GetMax();
                cout << "Saturation value too high. Set to maximum value" << endl;
            }
            else if (current_saturation_value < ptr_saturation->GetMin())
            {
                current_saturation_value = ptr_saturation->GetMin();
                cout << "Saturation value too low. Set to minimum value" << endl;
            }

            ptr_saturation->SetValue(current_saturation_value);
            cout << "Saturation set to " << ptr_saturation->GetValue() << endl;

            // Update the trackbar slider position based on the saturation value
            saturation_value_slider = static_cast<int>(current_saturation_value * 20);  // Reverse adjustment from 0.0 to 1.0 range to 0 to 20 range
        }
        else
        {
            cout << "Unable to enable saturation" << endl;
            return -1;
        }
        
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function controls gamma
int CAMERA_CONFIG::config_gamma(INodeMap& node_map, double current_gamma_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING GAMMA ***" << endl << endl;

    try
    {
        CBooleanPtr ptr_gamma_enable = node_map.GetNode("GammaEnable");    // Turn on gamma
        if (IsReadable(ptr_gamma_enable) && IsWritable(ptr_gamma_enable))
        {
            ptr_gamma_enable->SetValue(true);
            cout << "Gamma enabled" << endl;

            CFloatPtr ptr_gamma = node_map.GetNode("Gamma");
            if (!IsReadable(ptr_gamma) || !IsWritable(ptr_gamma))
            {
                cout << "Custom gamma format not readable or writable" << endl;
                return -1;
                
            }

            if (current_gamma_value > ptr_gamma->GetMax())
            {
                current_gamma_value = ptr_gamma->GetMax();
                cout << "Gamma value too high. Set to maximum value" << endl;
            }
            else if (current_gamma_value < ptr_gamma->GetMin())
            {
                current_gamma_value = ptr_gamma->GetMin();
                cout << "Gamma value too low. Set to minimum value" << endl;
            }

            ptr_gamma->SetValue(current_gamma_value);
            cout << "Gamma set to " << ptr_gamma->GetValue() << endl;

            // Update the trackbar slider position based on the gamma value
            gamma_value_slider = static_cast<int>((current_gamma_value - min_gamma) / (max_gamma - min_gamma) * gamma_slider_max_value);
        }
        else
        {
            cout << "Unable to enable gamma" << endl;
            return -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function controls sharpering
int CAMERA_CONFIG::config_sharpening(INodeMap& node_map, double current_sharpening_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING SHARPENING ***" << endl << endl;

    try
    {
        CBooleanPtr ptr_sharpening_enable = node_map.GetNode("SharpeningEnable");    // Turn on sharpening
        if (IsReadable(ptr_sharpening_enable) && IsWritable(ptr_sharpening_enable))
        {
            ptr_sharpening_enable->SetValue(true);
            cout << "Sharpening enabled" << endl;

            CFloatPtr ptr_sharpening = node_map.GetNode("Sharpening");
            if (!IsReadable(ptr_sharpening) || !IsWritable(ptr_sharpening))
            {
                cout << "Custom sharpness format not readable or writable" << endl;
                return -1;
            }

            if (current_sharpening_value > ptr_sharpening->GetMax())
            {
                current_sharpening_value = ptr_sharpening->GetMax();
                cout << "Sharpening value too high. Set to maximum value" << endl;
            }
            else if (current_sharpening_value < ptr_sharpening->GetMin())
            {
                current_sharpening_value = ptr_sharpening->GetMin();
                cout << "Sharpening value too low. Set to minimum value" << endl;
            }
            
            ptr_sharpening->SetValue(current_sharpening_value);
            cout << "Sharpening set to " << ptr_sharpening->GetValue() << endl;

            // Update the trackbar slider position based on the sharpening value
            sharpening_value_slider = static_cast<int>(current_sharpening_value + 1);  // Reverse adjustment from -1 to +8 range to 0 to 9
        }
        else
        {
            cout << "Unable to enable sharpening" << endl;
            return -1;
        }

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function controls gain
int CAMERA_CONFIG::config_gain(INodeMap& node_map, double current_gain_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING GAIN ***" << endl << endl;

    try
    {
        CEnumerationPtr ptr_gain_auto = node_map.GetNode("GainAuto");    // Turn off automatic gain
        if (IsReadable(ptr_gain_auto) && IsWritable(ptr_gain_auto))
        {
            CEnumEntryPtr ptr_gain_auto_off = ptr_gain_auto->GetEntryByName("Off");
            if (IsReadable(ptr_gain_auto_off))
            {
                ptr_gain_auto->SetIntValue(ptr_gain_auto_off->GetValue());
                cout << "Automatic gain disabled" << endl;
            }
        }
        else
        {
            cout << "Unable to disable automatic gain" << endl;
            return -1;
        }

        CFloatPtr ptr_gain = node_map.GetNode("Gain"); // Set gain manually, in dB
        if (!IsReadable(ptr_gain) || !IsWritable(ptr_gain))
        {
            cout << "Custom Gain format not readable or writable" << endl << endl;
            return -1;
        }

        if (current_gain_value > ptr_gain->GetMax())
        {
            current_gain_value = ptr_gain->GetMax();
            cout << "Gain value too high. Set to maximum value" << endl;
        }
        else if (current_gain_value < ptr_gain->GetMin())
        {
            current_gain_value = ptr_gain->GetMin();
            cout << "Gain value too low. Set to minimum value" << endl;
        }

        ptr_gain->SetValue(current_gain_value);
        cout << "Gain set to " << ptr_gain->GetValue() << endl;

        // Update the trackbar slider position based on the gain value
        gain_value_slider = static_cast<int>((current_gain_value - min_gain) / (max_gain - min_gain) * gain_slider_max_value);

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function configures pixel format
int CAMERA_CONFIG::config_pixel_format(INodeMap& node_map)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING PIXEL FORMAT ***" << endl << endl;

    try
    {
        // Configure pixel format
        CEnumerationPtr ptr_pixel_format = node_map.GetNode("PixelFormat");
        if (IsReadable(ptr_pixel_format) && IsWritable(ptr_pixel_format))
        {
            CEnumEntryPtr ptr_pixel_format_custom = ptr_pixel_format->GetEntryByName("BGR8"); // Custom pixel format name
            if (IsReadable(ptr_pixel_format_custom))
            {
                int64_t custom_pixel_format = ptr_pixel_format_custom->GetValue();
                ptr_pixel_format->SetIntValue(custom_pixel_format);

                cout << "Pixel format set to " << ptr_pixel_format->GetCurrentEntry()->GetSymbolic() << endl;
            }
            else
            {
                cout << "Your custom pixel format is not readable! Fix it!" << endl;
            }
        }
        else
        {
            cout << "Custom pixel format not readable or writable" << endl;
            return -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function configures the camera to use a custom region of interest (ROI) -> width, height, offset_x, offset_y
int CAMERA_CONFIG::config_roi(INodeMap& node_map, int64_t width_value, int64_t height_value, int64_t x_offset_value, int64_t y_offset_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING ROI: HEIGHT, WIDTH, OFFSET_X & OFFSET_Y ***" << endl << endl;

    try
    {
        CIntegerPtr offset_x_pointer = node_map.GetNode("OffsetX"); // Configure X offset
        if (IsReadable(offset_x_pointer) && IsWritable(offset_x_pointer))
        {
            if (x_offset_value >= offset_x_pointer->GetMin() && x_offset_value <= offset_x_pointer->GetMax())   // Ensure the value is within an acceptable range
            {
                offset_x_pointer->SetValue(x_offset_value);
                cout << "X offset set to " << offset_x_pointer->GetValue() << endl;
            }
            else
            {
                cout << "Offset X value out of range. Must be between " << offset_x_pointer->GetMin() << " and " << offset_x_pointer->GetMax() << endl;
            }
        }
        else
        {
            cout << "X offset not readable or writable" << endl;
            return -1;
        }

        CIntegerPtr offset_y_pointer = node_map.GetNode("OffsetY"); // Configure Y offset
        if (IsReadable(offset_y_pointer) && IsWritable(offset_y_pointer))
        {
            if (y_offset_value >= offset_y_pointer->GetMin() && y_offset_value <= offset_y_pointer->GetMax())   // Ensure the value is within an acceptable range
            {
                offset_y_pointer->SetValue(y_offset_value);
                cout << "X offset set to " << offset_y_pointer->GetValue() << endl;
            }
            else
            {
                cout << "Offset X value out of range. Must be between " << offset_y_pointer->GetMin() << " and " << offset_y_pointer->GetMax() << endl;
            }
        }
        else
        {
            cout << "X offset not readable or writable" << endl;
            return -1;
        }

        // Configure width
        CIntegerPtr width_pointer = node_map.GetNode("Width");
        if (IsReadable(width_pointer) && IsWritable(width_pointer))
        {
            if (width_value >= width_pointer->GetMin() && width_value <= width_pointer->GetMax())   // Ensure the value is within an acceptable range
            {
                width_pointer->SetValue(width_value);
                cout << "Width set to " << width_pointer->GetValue() << endl;
            }
            else
            {
                cout << "Width value out of range. Must be between " << width_pointer->GetMin() << " and " << width_pointer->GetMax() << endl;
            }
        }
        else
        {
            cout << "Width not readable or writable" << endl;
            return -1;
        }

        // Configure height
        CIntegerPtr height_pointer = node_map.GetNode("Height");
        if (IsReadable(height_pointer) && IsWritable(height_pointer))
        {
            if (height_value >= height_pointer->GetMin() && height_value <= height_pointer->GetMax())   // Ensure the value is within an acceptable range
            {
                height_pointer->SetValue(height_value);
                cout << "Height set to " << height_pointer->GetValue() << endl;
            }
            else
            {
                cout << "Height value out of range. Must be between " << height_pointer->GetMin() << " and " << height_pointer->GetMax() << endl;
            }
        }
        else
        {
            cout << "Height not readable or writable" << endl;
            return -1;
        }
    }
    catch(Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function configures a custom exposure time.
int CAMERA_CONFIG::config_exposure(INodeMap& node_map, double current_exposure_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING EXPOSURE ***" << endl << endl;

    try
    {
        CEnumerationPtr ptr_exposure_auto = node_map.GetNode("ExposureAuto");    // Turn off automatic exposure
        if (IsReadable(ptr_exposure_auto) && IsWritable(ptr_exposure_auto))
        {
            CEnumEntryPtr ptr_exposure_auto_ff = ptr_exposure_auto->GetEntryByName("Off");
            if (IsReadable(ptr_exposure_auto_ff))
            {
                ptr_exposure_auto->SetIntValue(ptr_exposure_auto_ff->GetValue());
                cout << "Automatic exposure disabled" << endl;
            }
        }
        else 
        {
            CEnumerationPtr ptr_auto_brightness = node_map.GetNode("autoBrightnessMode"); // Turn off auto brightness to use manual exposure
            if (!IsReadable(ptr_auto_brightness) || !IsWritable(ptr_auto_brightness))
            {
                cout << "Unable to get or set exposure time. Aborting" << endl << endl;
                return -1;
            }
            
            cout << "Unable to disable automatic exposure. Expected for some models" << endl;
            result = 1;
        }

        CFloatPtr ptr_exposure_time = node_map.GetNode("ExposureTime"); // Set exposure time manually, in microseconds
        if (!IsReadable(ptr_exposure_time) || !IsWritable(ptr_exposure_time))
        {
            cout << "Unable to get or set exposure time. Aborting" << endl << endl;
            return -1;
        }

        if (current_exposure_value > ptr_exposure_time->GetMax())
        {
            current_exposure_value = ptr_exposure_time->GetMax();
            cout << "Exposure value too high. Set to maximum value" << endl;
        }
        else if (current_exposure_value < ptr_exposure_time->GetMin())
        {
            current_exposure_value = ptr_exposure_time->GetMin();
            cout << "Exposure value too low. Set to minimum value" << endl;
        }

        ptr_exposure_time->SetValue(current_exposure_value);
        cout << std::fixed << "Exposure time set to " << ptr_exposure_time->GetValue() << " μs" << endl;

        // Update the trackbar slider position based on the exposure value
        exposure_value_slider = static_cast<int>((current_exposure_value - min_exposure) / (max_exposure-min_exposure) * exposure_slider_max_value);

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function returns the camera to its default state by re-enabling automatic exposure.
int CAMERA_CONFIG::reset_exposure(INodeMap& node_map)
{
    int result = 0;

    try
    {
        CEnumerationPtr ptr_exposure_auto = node_map.GetNode("ExposureAuto");
        if (!IsReadable(ptr_exposure_auto) || !IsWritable(ptr_exposure_auto))
        {
            cout << "Reset exposure is not not readable or writable. Non-fatal error" << endl << endl;
            return -1;
        }

        CEnumEntryPtr ptr_exposure_auto_continuous = ptr_exposure_auto->GetEntryByName("Continuous");
        if (!IsReadable(ptr_exposure_auto_continuous))
        {
            cout << "Unable to enable automatic exposure (enum entry retrieval). Non-fatal error" << endl << endl;
            return -1;
        }

        ptr_exposure_auto->SetIntValue(ptr_exposure_auto_continuous->GetValue());

        cout << "Automatic exposure enabled" << endl << endl;
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// Function to set terminal input mode (non-blocking)
void CAMERA_CONFIG::set_non_blocking_input(bool enable) 
{
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);

    if (enable)
    {
        ttystate.c_lflag &= ~ICANON; // Disable canonical mode
        ttystate.c_lflag &= ~ECHO;   // Disable echo
        ttystate.c_cc[VMIN] = 1;
    } 
    else 
    {
        ttystate.c_lflag |= ICANON;  // Enable canonical mode
        ttystate.c_lflag |= ECHO;    // Enable echo
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

// Function to get keyboard input
int CAMERA_CONFIG::keyboard_input()
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// This function acquires and saves images from the camera
int CAMERA_CONFIG::acquire_and_display_images(CameraPtr pointer_cam, INodeMap& node_map, INodeMap& node_map_tl_device)
{
    int result = 0;

    CAMERA_CONFIG camera_config; // Create an instance of class CAMERA_CONFIG
    camera_config.set_non_blocking_input(true); // Enable non-blocking mode

    bool running = true; // Running state of the camera

    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

    try
    {
        namedWindow("Display window", WINDOW_NORMAL); // Create window to display video
	    resizeWindow("Display window", camera_screen_width, camera_screen_height);	// Set custom height and width
        
        createTrackbar("Exposure", "Display window", &exposure_value_slider, exposure_slider_max_value, trackbar_callback_exposure, &pointer_cam); // Create trackbar for exposure
        createTrackbar("Gain", "Display window", &gain_value_slider, gain_slider_max_value, trackbar_callback_gain, &pointer_cam); // Create trackbar for gain
        createTrackbar("Sharpening", "Display window", &sharpening_value_slider, sharpening_slider_max_value, trackbar_callback_sharpening, &pointer_cam); // Create trackbar for sharpening
        createTrackbar("Gamma", "Display window", &gamma_value_slider, gamma_slider_max_value, trackbar_callback_gamma, &pointer_cam); // Create trackbar for gamma
        createTrackbar("Saturation", "Display window", &saturation_value_slider, saturation_slider_max_value, trackbar_callback_saturation, &pointer_cam); // Create trackbar for saturation
        
        CEnumerationPtr ptr_acquisition_mode = node_map.GetNode("AcquisitionMode");  // Setting acquisition mode to continuous
        if (IsReadable(ptr_acquisition_mode) && IsWritable(ptr_acquisition_mode))
        {
            CEnumEntryPtr ptr_acquisition_mode_continuous = ptr_acquisition_mode->GetEntryByName("Continuous");
            if (IsReadable(ptr_acquisition_mode_continuous))
            {
                const int64_t acquisition_mode_continuous = ptr_acquisition_mode_continuous->GetValue();

                ptr_acquisition_mode->SetIntValue(acquisition_mode_continuous);

                cout << "Acquisition mode set to continuous" << endl;

                pointer_cam->BeginAcquisition();

                cout << "Acquiring images" << endl;

                CFloatPtr ptr_exposure_time = node_map.GetNode("ExposureTime"); // Get the value of exposure time to set an appropriate timeout for GetNextImage
                if (IsReadable(ptr_exposure_time) && IsWritable(ptr_exposure_time))
                {
                    // The exposure time is retrieved in µs so it needs to be converted to ms to keep consistency with the unit
                    uint64_t timeout = static_cast<uint64_t>(ptr_exposure_time->GetValue() / 1000 + 1000);

                    ImageProcessor processor;   // Create image processor instance for post processing images

                    processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_DIRECTIONAL_FILTER);  // Set interpolation algorithm

                    while (running)
                    {
                        try
                        {
                            ImagePtr p_result_image_pointer = pointer_cam->GetNextImage(timeout);  // To ensure that an image has enough time to arrive 

                            if (p_result_image_pointer->IsIncomplete())
                            {
                                cout << "Image incomplete with image status " << p_result_image_pointer->GetImageStatus() << endl;
                            }
                            else
                            {
                                // Convert image to custom color processing algorithm
                                ImagePtr converted_image = processor.Convert(p_result_image_pointer, PixelFormat_BGR8); // Convert to BGR format

                                // Convert image to OpenCV format
                                size_t width = converted_image->GetWidth();
                                size_t height = converted_image->GetHeight();
                                Mat image = Mat(height, width, CV_8UC3, converted_image->GetData(), Mat::AUTO_STEP);

                                waitKey(50);  // Wait for ms
                                imshow("Display window", image);    // Display image

                                if(!image.empty())
                                {
                                    if (camera_config.keyboard_input())
                                    {
                                        int key = getchar();
                                        if(key == 'q' || key == 'Q') // If the user presses 'q', exit the loop
                                        {
                                            running = false; // Stop the loop
                                            save_data_to_database(); // Save data to database
                                        }
                                    }
                                }
                                else
                                {
                                    cout << "Image empty" << endl;
                                }
                            }
                        }
                        catch (Spinnaker::Exception &e)
                        {
                            cout <<  "Error: " << e.what() << endl;
                            result = -1;
                        }
                    }
                    pointer_cam->EndAcquisition();  // End acquisition
                    camera_config.set_non_blocking_input(false);   // Set input to blocking mode
                    destroyAllWindows();
                }
                else
                {
                    cout << "Unable to get or set exposure time. Aborting" << endl << endl;
                    return -1;
                }
            }
            else
            {
                cout << "Unable to get or set acquisition mode to continuous (entry retrieval). Aborting" << endl << endl;
                return -1;
            }
	    }
        else
        {
            cout << "Unable to get or set acquisition mode to continuous (node retrieval). Aborting." << endl << endl;
            return -1;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function acts as the main function for the camera configuration
int CAMERA_CONFIG::run_single_camera(CameraPtr pointer_cam)
{
    int result = 0;

    try
    {   
        cout << "Running single camera configuration" << endl;
        INodeMap& node_map_tl_device = pointer_cam->GetTLDeviceNodeMap();   // Retrieve TL device nodemap and print device information

        cout << "Initialize camera \n" << endl;
        pointer_cam->Init();    // Initialize camera

        INodeMap& node_map = pointer_cam->GetNodeMap(); // Retrieve GenICam nodemap

        node_map.GetNode("DeviceReset");    // Accessing the node map to ensure it's refreshed

        cout << "Running pixel format function" << endl;
        result = result | CAMERA_CONFIG::config_pixel_format(node_map); // Pixel Format

        cout << "Running ROI function" << endl;
        result = result | CAMERA_CONFIG::config_roi(node_map, camera_screen_height, camera_screen_width, 0, 0); // Width, Height, X_offset, Y_offset [pixels]

        cout << "Setting initial exposure" << endl;
        result = result | CAMERA_CONFIG::config_exposure(node_map, exposure_value);  // Exposure Time

        cout << "Setting initial gain" << endl;
        result = result | CAMERA_CONFIG::config_gain(node_map, gain_value);  // Gain

        cout << "Setting initial sharpening" << endl;
        result = result | CAMERA_CONFIG::config_sharpening(node_map, sharpening_value);  // Sharpening

        cout << "Setting initial gamma" << endl;
        result = result | CAMERA_CONFIG::config_gamma(node_map, gamma_value);  // Gamma

        cout << "Setting initial saturation" << endl;
        result = result | CAMERA_CONFIG::config_saturation(node_map, saturation_value);  // Saturation
        
        cout << "Running acquire images function" << endl;
        result = result | CAMERA_CONFIG::acquire_and_display_images(pointer_cam, node_map, node_map_tl_device); // Calling out acquire_and_display_images function and checking if it returns 0   

        if (result == 0)
        {
            cout << "Running reset exposure function" << endl;
            result = result | CAMERA_CONFIG::reset_exposure(node_map);
        }
        else
        {
            cout << "Skipping exposure reset" << endl << endl;
        }

        cout << "Deinitialize camera \n" << endl;
        pointer_cam->DeInit();  // Deinitialize camera
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

int main(int argc, char** argv)
{
    int result = 0;

    SystemPtr system = System::GetInstance();

    CameraList camera_list = system->GetCameras();

    unsigned int num_cameras = camera_list.GetSize();

    cout << "Number of cameras detected: " << num_cameras << endl << endl;

    if (num_cameras == 0)   // Finish if there are no cameras
    {
        camera_list.Clear();    // Release camera list before releasing system
        system->ReleaseInstance();  // Release system

        cout << "Not enough cameras!" << endl;
        cout << "Done! Press Enter to exit" << endl;
        getchar();

        return -1;
    }

    CAMERA_CONFIG camera_config; // Create camera instance because we are using class functions and not static functions

    for (unsigned int i = 0; i < num_cameras; i++)  // Run configuration on each camera
    {
        cout << "Running configuration for camera " << i << endl;

        result = result | camera_config.run_single_camera(camera_list.GetByIndex(i));

        cout << "Camera " << i << " configuration complete" << endl;
    }

    camera_list.Clear();    // Release camera list before releasing system

    system->ReleaseInstance();  // Release system

    cout << "Done! Press Enter to exit" << endl;
    getchar();

    return result;
}
