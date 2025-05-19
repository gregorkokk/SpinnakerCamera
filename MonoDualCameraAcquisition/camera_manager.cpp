// Description: This file contains the implementation of the camera manager class -> setting up the cameras and acquiring images
// Author: Gregor Kokk
// Date: 06.01.2025

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>
#include <atomic>

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "camera_manager.h"
#include "camera_settings.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

/**
 * Constructor for the CAMERA_MANAGER class.
 * @param settings: The camera settings object to use for configuration.
 * @return A new CAMERA_MANAGER instance.
 */
CAMERA_MANAGER::CAMERA_MANAGER(const CAMERA_SETTINGS* settings)
    : camera_settings(settings) // Initialize the camera_settings pointer
{
    if (!camera_settings)
    {
        throw invalid_argument("camera_settings pointer cannot be null.");
    }
}

/**
 * Destructor for the CAMERA_MANAGER class.
 * @param None: No input parameters.
 * @return None: No return value.
 */
CAMERA_MANAGER::~CAMERA_MANAGER() {}

/**
 * Configures Black Level Clamping Enable for the cameras.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_black_level_clamping_enable(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING BLACK LEVEL CLAMPING ENABLE ***" << endl << endl;

    try
    {
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                // Access the BlackLevelClampingEnable node
                CBooleanPtr ptr_black_level_clamping_enable = node_maps[i]->GetNode("BlackLevelClampingEnable");
                if (!IsReadable(ptr_black_level_clamping_enable) || !IsWritable(ptr_black_level_clamping_enable))
                {
                    cout << "Unable to enable black level clamping for Camera " << i << ". Skipping.\n";
                    continue;
                }

                // Apply black level clamping to the camera
                ptr_black_level_clamping_enable->SetValue(true);
                cout << "[Camera" << i << "] Black level clamping set to: " << ptr_black_level_clamping_enable->GetValue() << "\n";
            }
            catch (const Spinnaker::Exception& e)
            {
                cerr << "Error in black level clamping loop for Camera " << i << ": " << e.what() << endl;
                result = -1;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cerr << "ERROR IN CONFIGURING BLACK LEVEL CLAMPING ENABLE: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Configures Sensor Shutter Mode for the cameras.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_sensor_shutter_mode(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING SENSOR SHUTTER MODE ***" << endl << endl;

    try
    {
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                // Access the SensorShutterMode node
                CEnumerationPtr ptr_sensor_shutter_mode = node_maps[i]->GetNode("SensorShutterMode");
                if (!IsReadable(ptr_sensor_shutter_mode) || !IsWritable(ptr_sensor_shutter_mode))
                {
                    cout << "Unable to set sensor shutter mode for Camera " << i << ". Skipping.\n";
                    continue;
                }

                CEnumEntryPtr ptr_sensor_shutter_mode_global = ptr_sensor_shutter_mode->GetEntryByName("Global");
                if(IsReadable(ptr_sensor_shutter_mode_global))
                {
                    int64_t custom_sensor_shutter_mode = ptr_sensor_shutter_mode_global->GetValue();
                    ptr_sensor_shutter_mode->SetIntValue(custom_sensor_shutter_mode);
                    
                    cout << "[Camera " << i << "] Sensor shutter mode set to: " << ptr_sensor_shutter_mode->GetCurrentEntry()->GetSymbolic()  << endl;
                }
            }
            catch (const Spinnaker::Exception& e)
            {
                cerr << "Error in to set sensor shutter to Global, in camera: " << i << ": " << e.what() << endl;
                result = -1;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cerr << "ERROR IN CONFIGURING SENSOR SHUTTER MODE: " << e.what() << endl;
        result = -1;
    }
    

    return result;
}

/**
 * Configures gain for the cameras.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_gain(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING GAIN ***" << endl << endl;

    try
    {
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                // Turn off automatic gain
                CEnumerationPtr ptr_gain_auto = node_maps[i]->GetNode("GainAuto");
                if (!IsReadable(ptr_gain_auto) || !IsWritable(ptr_gain_auto))
                {
                    cout << "Unable to disable automatic gain for Camera " << i << ". Skipping.\n";
                    continue; // Skip this camera
                }

                CEnumEntryPtr ptr_gain_auto_off = ptr_gain_auto->GetEntryByName("Off");
                if (IsReadable(ptr_gain_auto_off))
                {
                    ptr_gain_auto->SetIntValue(ptr_gain_auto_off->GetValue());
                    cout << "[Camera " << i << "] Automatic gain disabled. \n";
                }

                // Get and set manual gain
                CFloatPtr ptr_gain = node_maps[i]->GetNode("Gain");
                if (!IsReadable(ptr_gain) || !IsWritable(ptr_gain))
                {
                    cout << "[Camera " << i << "] Unable to get or set gain. Skipping. \n";
                    continue; // Skip this camera
                }

                // Retrieve and validate gain value
                double gain_value = camera_settings->get_gain();

                if (gain_value > ptr_gain->GetMax())
                {
                    gain_value = ptr_gain->GetMax();
                    cout << "Gain value too high. Set to maximum value: " << gain_value << endl;
                }
                else if (gain_value < ptr_gain->GetMin())
                {
                    gain_value = ptr_gain->GetMin();
                    cout << "Gain value too low. Set to minimum value: " << gain_value << endl;
                }

                // Apply gain to the camera
                ptr_gain->SetValue(gain_value);
                cout << "[Camera " << i << "] Gain set to: " << ptr_gain->GetValue() << endl;
            }
            catch (const std::exception& e)
            {
                cerr << "[Camera " << i << "] Error in gain loop: " << e.what() << endl;
                result = -1;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "ERROR IN CONFIGURING GAIN: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Configures gamma for the cameras.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_gamma(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING GAMMA ***" << endl << endl;

    try
    {
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                // Turn on gamma
                CBooleanPtr ptr_gamma_enable = node_maps[i]->GetNode("GammaEnable");
                if (!IsReadable(ptr_gamma_enable) || !IsWritable(ptr_gamma_enable))
                {
                    cout << "Unable to enable gamma for Camera " << i << ". Skipping.\n";
                    continue;
                }
                ptr_gamma_enable->SetValue(true);
                cout << "[Camera " << i << "] Gamma enabled. \n";

                // Set gamma manually
                CFloatPtr ptr_gamma = node_maps[i]->GetNode("Gamma");
                if (!IsReadable(ptr_gamma) || !IsWritable(ptr_gamma))
                {
                    cout << "[Camera " << i << "] Unable to get or set gamma. Skipping.\n";
                    continue;
                }

                double gamma_value = camera_settings->get_gamma();  // Retrieve and validate gamma value

                if (gamma_value > ptr_gamma->GetMax())
                {
                    gamma_value = ptr_gamma->GetMax();
                    cout << "Gamma value too high. Set to maximum value: " << gamma_value << endl;
                }
                else if (gamma_value < ptr_gamma->GetMin())
                {
                    gamma_value = ptr_gamma->GetMin();
                    cout << "Gamma value too low. Set to minimum value: " << gamma_value << endl;
                }

                ptr_gamma->SetValue(gamma_value);   // Apply gamma to the camera
                cout << "[Camera " << i << "] Gamma set to: " << ptr_gamma->GetValue() << endl;
            }
            catch (const std::exception& e)
            {
                cerr << "[Camera " << i << "] Error in gamma loop: " << e.what() << endl;
                result = -1;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "ERROR IN CONFIGURING GAMMA: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Configures exposure time for the cameras.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_exposure(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING EXPOSURE ***" << endl << endl;

    try
    {
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                // Turn off automatic exposure
                CEnumerationPtr ptr_exposure_auto = node_maps[i]->GetNode("ExposureAuto");
                if (IsReadable(ptr_exposure_auto) && IsWritable(ptr_exposure_auto))
                {
                    CEnumEntryPtr ptr_exposure_auto_off = ptr_exposure_auto->GetEntryByName("Off");
                    if (IsReadable(ptr_exposure_auto_off))
                    {
                        ptr_exposure_auto->SetIntValue(ptr_exposure_auto_off->GetValue());
                        cout << "[Camera " << i << "] Automatic exposure disabled for Camera " << i << endl;
                    }
                }
                else
                {
                    cout << "Unable to disable automatic exposure for Camera " << i << ". Skipping.\n";
                    continue;
                }

                // Set exposure time manually
                CFloatPtr ptr_exposure_time = node_maps[i]->GetNode("ExposureTime");
                if (!IsReadable(ptr_exposure_time) || !IsWritable(ptr_exposure_time))
                {
                    cout << "Unable to get or set exposure time for Camera " << i << ". Skipping.\n";
                    continue;
                }

                // Retrieve and validate exposure value
                double exposure_value = camera_settings->get_exposure();

                if (exposure_value > ptr_exposure_time->GetMax())
                {
                    exposure_value = ptr_exposure_time->GetMax();
                    cout << "Exposure value too high. Set to maximum value: " << exposure_value << endl;
                }
                else if (exposure_value < ptr_exposure_time->GetMin())
                {
                    exposure_value = ptr_exposure_time->GetMin();
                    cout << "Exposure value too low. Set to minimum value: " << exposure_value << endl;
                }

                ptr_exposure_time->SetValue(exposure_value);    // Apply exposure to the camera
                cout << "[Camera " << i << "] Exposure set to: " << ptr_exposure_time->GetValue() << " μs" << endl;
            }
            catch (const std::exception& e)
            {
                cerr << "Error in exposure loop for Camera " << i << ": " << e.what() << endl;
                result = -1;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "ERROR IN CONFIGURING EXPOSURE: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Configures the camera to its default state by re-enabling automatic exposure.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::reset_exposure(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << "\n\n*** RESET EXPOSURE ***\n\n";

    try
    {    
        // Change the exposure time for each camera
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                CEnumerationPtr ptr_exposure_auto = node_maps[i]->GetNode("ExposureAuto");
                if (!IsReadable(ptr_exposure_auto) || !IsWritable(ptr_exposure_auto))
                {
                    cout << "Reset exposure is not not readable or writable. Non-fatal error" << endl << endl;
                    continue;
                    return -1;
                }

                CEnumEntryPtr ptr_exposure_auto_continuous = ptr_exposure_auto->GetEntryByName("Continuous");
                if (!IsReadable(ptr_exposure_auto_continuous))
                {
                    cout << "Unable to enable automatic exposure (enum entry retrieval). Non-fatal error" << endl << endl;
                    continue;
                    return -1;
                }

                ptr_exposure_auto->SetIntValue(ptr_exposure_auto_continuous->GetValue());
                cout << "[Camera " << i << "] Automatic exposure enabled\n";
            }
            catch(const std::exception& e)
            {
                cerr << "Error in reset exposure loop: " << e.what() << endl;
            }
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "ERROR IN RESET EXPOSURE: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Configures pixel format for the cameras.
 * @param node_maps: The GenICam node maps for the cameras.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_pixel_format(const std::vector<INodeMap*>& node_maps)
{
    int result = 0;

    cout << "\n\n*** CONFIGURING PIXEL FORMAT ***\n\n";

    try
    {
        for (unsigned int i = 0; i < node_maps.size(); i++)
        {
            try
            {
                // Configure pixel format
                CEnumerationPtr ptr_pixel_format = node_maps[i]->GetNode("PixelFormat");
                if (!IsReadable(ptr_pixel_format) || !IsWritable(ptr_pixel_format))
                {
                    cout << "Unable to set pixel format for Camera " << i << ". Skipping.\n";
                    continue;
                }

                CEnumEntryPtr ptr_pixel_format_custom = ptr_pixel_format->GetEntryByName("Mono16");
                if (IsReadable(ptr_pixel_format_custom))
                {
                    int64_t custom_pixel_format = ptr_pixel_format_custom->GetValue();
                    ptr_pixel_format->SetIntValue(custom_pixel_format);

                    cout << "[Camera " << i << "] Pixel format set to " << ptr_pixel_format->GetCurrentEntry()->GetSymbolic() << endl;
                }
                else
                {
                    cout << "Pixel format not readable. Skipping.\n";
                }
            }
            catch (const std::exception& e)
            {
                cerr << "Error in pixel format loop for Camera " << i << ": " << e.what() << endl;
                result = -1;
            }
        }
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "ERROR IN CONFIGURING PIXEL FORMAT: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Configures the camera to use a custom region of interest (ROI) -> width, height, OffsetX, and OffsetY.
 * @param node_map: The GenICam node map for the camera.
 * @param offset_x: The OffsetX value for the region.
 * @param width: The width value for the region.
 * @param height: The height value for the region.
 * @param camera_index: The index of the camera.
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::config_roi(INodeMap* node_map, int64_t offset_x, int64_t offset_y, int64_t width, int64_t height, unsigned int camera_index)
{
    int result = 0;

    cout << "\n\n*** CONFIGURING ROI: HEIGHT, WIDTH, OFFSET-X & OFFSET-Y ***\n\n";

    try
    {
        // Configure Width
        CIntegerPtr width_pointer = node_map->GetNode("Width");
        if (IsReadable(width_pointer) && IsWritable(width_pointer))
        {
            cout << "[Camera " << camera_index << "] Width range: " << width_pointer->GetMin() << " to " << width_pointer->GetMax() << endl;
            if (width >= width_pointer->GetMin() && width <= width_pointer->GetMax())
            {
                width_pointer->SetValue(width);	// Apply width
                cout << "[Camera " << camera_index << "] Width set to " << width_pointer->GetValue() << endl;
            }
            else
            {
                cerr << "[Camera " << camera_index << "] Width value out of range. Must be between " << width_pointer->GetMin() << " and " << width_pointer->GetMax() << endl;
            }
        }
        else
        {
            cerr << "[Camera " << camera_index << "] Width not readable or writable. Skipping.\n";
        }
        
        // Configure OffsetX
        CIntegerPtr ptr_offsetX = node_map->GetNode("OffsetX");
        if (IsReadable(ptr_offsetX) && IsWritable(ptr_offsetX))
        {
            cout << "[Camera " << camera_index << "] OffsetX range: " << ptr_offsetX->GetMin() << " to " << ptr_offsetX->GetMax() << endl;
            if (offset_x <= ptr_offsetX->GetMax() && offset_x >= ptr_offsetX->GetMin())
            {
                ptr_offsetX->SetValue(offset_x);    // Apply offset_x
                cout << "[Camera " << camera_index << "] OffsetX set to " << ptr_offsetX->GetValue() << endl;
            }
            else
            {
                cerr << "[Camera " << camera_index << "] OffsetX value out of range. Must be between " << ptr_offsetX->GetMin() <<  " and " <<ptr_offsetX->GetMax() << endl;
            }
        }
        else
        {
            cerr << "[Camera " << camera_index << "] OffsetX not readable or writable. Skipping.\n";
        }

        // Configure Height
        CIntegerPtr height_pointer = node_map->GetNode("Height");
        if (IsReadable(height_pointer) && IsWritable(height_pointer))
        {
            if (height >= height_pointer->GetMin() && height <= height_pointer->GetMax())
            {
                height_pointer->SetValue(height); // Apply height
                cout << "[Camera " << camera_index << "] Height set to " << height_pointer->GetValue() << endl;
            }
            else
            {
                cerr << "[Camera " << camera_index << "] Height value out of range. Must be between " << height_pointer->GetMin() << " and " << height_pointer->GetMax() << endl;
                result = -1;
            }
        }
        else
        {
            cerr << "[Camera " << camera_index << "] Height not readable or writable. Skipping.\n";
        }

        // Configure OffsetY
        CIntegerPtr ptr_offsetY = node_map->GetNode("OffsetY");
        if (IsReadable(ptr_offsetY) && IsWritable(ptr_offsetY))
        {
            if (offset_y <= ptr_offsetY->GetMax() && offset_y >= ptr_offsetY->GetMin())
            {
                ptr_offsetY->SetValue(offset_y); // Apply offset_y
                cout << "[Camera " << camera_index << "] OffsetY set to " << ptr_offsetY->GetValue() << endl;
            }
            else
            {
                cerr << "[Camera " << camera_index << "] OffsetY value out of range. Must be between " << ptr_offsetY->GetMin() << " and " << ptr_offsetY->GetMax() << endl;
            }
        }
        else
        {
            cerr << "[Camera " << camera_index << "] OffsetY not readable or writable. Skipping.\n";
        }

        // Debugging: Log the applied settings
        /*
        cout << "\n[Camera " << camera_index << "] Verified ROI Settings:\n";
        cout << "    Width: " << width_pointer->GetValue() << endl;
        cout << "    OffsetX: " << ptr_offsetX->GetValue() << endl;
        cout << "    Height: " << height_pointer->GetValue() << endl;
        cout << "    OffsetY: " << ptr_offsetY->GetValue() << endl;
        */
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "Error during ROI configuration: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Function to set terminal input mode (non-blocking)
 * @param enable: True to enable non-blocking input, false to disable.
 */
void CAMERA_MANAGER::set_non_blocking_input(bool enable)
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
int CAMERA_MANAGER::keyboard_input()
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

/**
 * Captures an image for a specific region based on OffsetX.
 * @param camera: The camera to capture the image.
 * @param timeout: The timeout for image acquisition.
 * @param folder_path: The folder path to save the image.
 * @param image_prexif: The prefix for the image filename.
 * @param device_serial: The serial number of the camera for the filename.
 * @param image_count: The current image count for the filename.
 * @param camera_index: The index of the camera.
 * @param offset_x: The current offset_x value for the region.
 */
void CAMERA_MANAGER::capture_image(
    CameraPtr& camera,
    uint64_t timeout,
    const string& folder_path,
    const string& device_serial,
    unsigned int image_index,
    unsigned int camera_index,
    int64_t offset_x)
{
    cout << "\n\n*** CAPTURING IMAGE FOR CAMERA ***\n\n";

    try
    {
        ImagePtr image_ptr = camera->GetNextImage(timeout);
        if (image_ptr->IsIncomplete())
        {
            cerr << "[Camera " << camera_index << "] Incomplete image captured\n";
            image_ptr->Release();
            return;
        }

        // Convert the image to Mono16 format
        ImageProcessor processor;
        ImagePtr converted_image = processor.Convert(image_ptr, PixelFormat_Mono16);

        // Build the filename: <prefix>_Serial_<serial>_OffsetX_<offset_x>_Image_<circular_index>.jpg
        unsigned int circular_index = image_index % 3; // Limit to 5 images per offset
        string full_filename = folder_path + "Serial_" + device_serial +
                               "_OffsetX_" + to_string(offset_x) + "_Image_" + to_string(circular_index) + ".jpg";

        // Save the image
        converted_image->Save(full_filename.c_str());
        cout << "[Camera " << camera_index << "] Image saved at: " << full_filename << endl;

        image_ptr->Release();
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "[Camera " << camera_index << "] Error capturing image: " << e.what() << endl;
    }
}

/**
 * Handles cleanup after image acquisition.
 * @param cameras: The camera pointers to clean up.
 */
void CAMERA_MANAGER::stop_camera_acquisition(vector<CameraPtr>& cameras)
{
    cout << "\n\n*** ENDING ACQUISITION ***\n\n";

    size_t stopped_count = 0;

    for (size_t i = 0; i < cameras.size(); ++i)
    {
        CameraPtr& camera = cameras[i];
        if (!camera)
        {
            cout << "[Camera " << i << "] Camera pointer is null. Skipping.\n";
            continue;
        }

        if (!camera->IsInitialized())
        {
            cout << "[Camera " << i << "] Camera is not initialized. Skipping.\n";
            continue;
        }

        try
        {
            if (camera->IsStreaming()) // Ensure the camera is streaming
            {
                camera->EndAcquisition();
                cout << "[Camera " << i << "] Acquisition stopped successfully.\n";
                ++stopped_count;
            }
            else
            {
                cout << "[Camera " << i << "] Camera is not streaming. Skipping.\n";
            }
        }
        catch (const Spinnaker::Exception& e)
        {
            cerr << "[Camera " << i << "] Error stopping acquisition: " << e.what() << endl;
        }
    }

    cout << "\n*** STOPPING ACQUISITION COMPLETE: " << stopped_count << " out of " << cameras.size() << " cameras stopped successfully. ***\n";
}

/**
 * Handles keyboard interrupts during image acquisition.
 * 
 * @return true if the acquisition should stop, false otherwise.
 */
bool CAMERA_MANAGER::handle_keyboard_interrupt()
{
    char key = getchar();
    if (key == 'q' || key == 'Q')
    {
        cout << endl << "\n\n*** KEYBOARD INTERRUPT DETECTED ***\n\n";
        cout << "Keyboard interrupt detected. Stopping image acquisition...\n";
        set_non_blocking_input(false);  // Restore blocking input mode
        return true;  // Signal to stop the acquisition
    }

    return false;  // Continue the acquisition loop
}

/**
 * Sets the acquisition mode of the camera to "Continuous".
 * @param node_map: The GenICam node map for the camera.
 * @param camera_index: The index of the camera (for logging purposes).
 * @return 0 if successful, -1 if an error occurred during configuration.
 */
int CAMERA_MANAGER::set_acquisition_mode(INodeMap* node_map, unsigned int camera_index)
{
    int result = 0;

    try
    {
        // Retrieve the acquisition mode node
        CEnumerationPtr ptr_acquisition_mode = node_map->GetNode("AcquisitionMode");
        if (!IsReadable(ptr_acquisition_mode) || !IsWritable(ptr_acquisition_mode))
        {
            cerr << "[Camera " << camera_index << "] Unable to access or set AcquisitionMode. Skipping.\n";
            result = -1;
        }

        // Retrieve the "Continuous" mode entry
        CEnumEntryPtr ptr_acquisition_mode_continuous = ptr_acquisition_mode->GetEntryByName("Continuous");
        if (!IsReadable(ptr_acquisition_mode_continuous))
        {
            cerr << "Camera " << camera_index << ": Continuous acquisition mode is not readable. Skipping.\n";
            result = -1;
        }

        // Set the acquisition mode to "Continuous"
        const int64_t acquisition_mode_continuous = ptr_acquisition_mode_continuous->GetValue();
        ptr_acquisition_mode->SetIntValue(acquisition_mode_continuous);

        cout << "[Camera " << camera_index << "] Acquisition mode set to Continuous.\n";
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "[Camera " << camera_index << "] Error setting acquisition mode: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Starts acquisition for the given camera.
 * @param camera: The camera pointer to start acquisition.
 * @param camera_index: The index of the camera (for logging purposes).
 * @return true if acquisition started successfully, false otherwise.
 */
int CAMERA_MANAGER::start_camera_acquisition(CameraPtr& camera, unsigned int camera_index)
{
    int result = 0;

    try
    {
        camera->BeginAcquisition();
        cout << "[Camera " << camera_index << "] Acquisition started.\n";
        result = -1;
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "[Camera " << camera_index << "] Error starting acquisition: " << e.what() << endl;
        result = -1;
    }

    return result;
}

/**
 * Calculates the timeout value for image acquisition based on the camera's exposure time.
 * 
 * @param node_map: The GenICam node map for the camera.
 * @param camera_index: The index of the camera (for logging purposes).
 * @return The calculated timeout in milliseconds.
 */
uint64_t CAMERA_MANAGER::calculate_exposure_timeout(INodeMap* node_map, unsigned int camera_index)
{
    try
    {
        // Get the ExposureTime node
        CFloatPtr exposure_ptr = node_map->GetNode("ExposureTime");
        if (IsReadable(exposure_ptr))
        {
            // Convert exposure time to milliseconds
            uint64_t exposure_time = static_cast<uint64_t>(exposure_ptr->GetValue() / 1000);

            // Small dynamic buffer based on exposure time (e.g., 10% of exposure time or a minimum of 10 ms)
            uint64_t buffer_time = std::max<uint64_t>(10, exposure_time / 10);

            uint64_t total_timeout = exposure_time + buffer_time;
            cout << "[Camera " << camera_index << "] Calculated timeout = " << total_timeout 
                 << " ms (Exposure = " << exposure_time << " ms + Buffer = " << buffer_time << " ms).\n\n";
            return total_timeout;
        }
        else
        {
            cerr << "[Camera " << camera_index << "] ExposureTime node is not readable. Using default minimal timeout.\n";
            return 100;  // Minimal default timeout
        }
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "[Camera " << camera_index << "] Error retrieving ExposureTime: " << e.what() << endl;
        return 100;  // Minimal default timeout on error
    }
}

/**
 * Retrieves the serial number of a camera.
 * @param node_map_tl_device: The GenICam Transport Layer (TL) device node map.
 * @param camera_index: The index of the camera (for logging purposes).
 * @return The serial number of the camera as a string, or an empty string if retrieval fails.
 */
string CAMERA_MANAGER::get_camera_serial_number(INodeMap* node_map_tl_device, unsigned int camera_index)
{
    try
    {
        // Retrieve the DeviceSerialNumber node
        CStringPtr serial_ptr = node_map_tl_device->GetNode("DeviceSerialNumber");
        if (serial_ptr && IsReadable(serial_ptr))
        {
            // Convert gcstring to string
            string serial_number = serial_ptr->GetValue().c_str();
            cout << "[Camera " << camera_index << "] Serial number = " << serial_number << endl;
            return serial_number;
        }
        else
        {
            cerr << "[Camera " << camera_index << "] DeviceSerialNumber node is not readable.\n";
            return "";  // Return an empty string if the serial number is not readable
        }
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "[Camera " << camera_index << "] Error retrieving serial number: " << e.what() << endl;
        return "";  // Return an empty string on error
    }
}

/**
 * Combines a base folder path and a subpath, ensuring a valid file path.
 * 
 * @param base: The base folder path.
 * @param subpath: The subpath to append to the base folder path.
 * @return: A valid combined file path.
 */
string combine_path(const string& base, const string& subpath)
{
    if (base.empty())
    {
        return subpath; // If the base path is empty, just return the subpath
    }

    ostringstream result;
    result << base; // Add the base path

    // Ensure the base ends with a single slash
    if (base.back() != '/')
    {
        result << '/';
    }

    // Add the subpath, removing any leading slash
    if (!subpath.empty() && subpath.front() == '/')
    {
        result << subpath.substr(1);
    }
    else
    {
        result << subpath;
    }

    return result.str();
}

/**
 * Checks if a camera and its node map are valid.
 * @param camera: The camera pointer to check.
 * @param node_map: The GenICam node map to check.
 * @param camera_index: The index of the camera (for logging purposes).
 * @return true if the camera and node map are valid, false otherwise.
 */
bool CAMERA_MANAGER::is_camera_valid(const CameraPtr& camera, INodeMap* node_map, unsigned int camera_index)
{
    if (!camera || !node_map)
    {
        cerr << "[Camera " << camera_index << "] is invalid. Skipping.\n";
        return false;
    }
    return true;
}

/**
 * Acquires images from multiple cameras.
 * 
 * @param cameras: Vector of camera pointers to acquire images from.
 * @param number_of_cameras: The number of cameras to acquire images from.
 * @param node_maps: Vector of GenICam node maps for the cameras.
 * @param node_maps_tl_device: Vector of GenICam TL device node maps for the cameras.
 * @param global_running: The global running flag to stop acquisition.
 * @param folder_path: The base folder path where images will be saved.
 * @return 0 if successful, -1 if an error occurred during acquisition.
 */
int CAMERA_MANAGER::acquire_images(
    vector<CameraPtr>& cameras,
    unsigned int number_of_cameras,
    const vector<INodeMap*>& node_maps,
    const vector<INodeMap*>& node_maps_tl_device,
    atomic<bool>& global_running,
    const string& folder_path)
{
    set_non_blocking_input(true);

    int result = 0;
    bool local_running = true;

    cout << "\n\n*** IMAGE ACQUISITION ***\n\n";

    vector<string> device_serial_numbers(number_of_cameras, "");
    vector<uint64_t> timeouts(number_of_cameras, 1000);
    vector<map<int64_t, unsigned int>> image_counts(number_of_cameras); // Track image counts for each offset_x

    try
    {
        // Prepare each camera
        for (unsigned int i = 0; i < number_of_cameras; i++)
        {
            string serial_number = get_camera_serial_number(node_maps_tl_device[i], i);
            if (serial_number.empty())
            {
                cerr << "[Camera " << i << "] Failed to retrieve serial number. Skipping.\n";
                continue;
            }
            device_serial_numbers[i] = serial_number;
            timeouts[i] = calculate_exposure_timeout(node_maps[i], i);
        }

        // Main acquisition loop
        while (local_running && global_running.load())
        {
            for (unsigned int i = 0; i < number_of_cameras; i++) // Loop over cameras
            {
                if (!is_camera_valid(cameras[i], node_maps[i], i))
                    continue;

                for (const auto& roi : roi_config_values) // Alternate offsets for each camera
                {
                    // Calculate circular index for overwriting
                    unsigned int circular_index = image_counts[i][roi.offset_x] % 5;

                    // Apply ROI
                    cout << "Applying ROI for Camera " << i
                         << " - OffsetX: " << roi.offset_x
                         << ", OffsetY: " << roi.offset_y
                         << ", Width: " << roi.width
                         << ", Height: " << roi.height << endl;

                    result |= config_roi(node_maps[i], roi.offset_x, roi.offset_y, roi.width, roi.height, i);

                    // Start acquisition
                    result |= set_acquisition_mode(node_maps[i], i);
                    result |= start_camera_acquisition(cameras[i], i);

                    try
                    {
                        // Capture the image
                        //string camera_folder = combine_path(folder_path, "camera_" + to_string(i));
                        capture_image(
                            cameras[i],
                            timeouts[i],
                            folder_path,
                            device_serial_numbers[i],
                            circular_index, // Circular index to overwrite images
                            i,
                            roi.offset_x
                        );
                        cout << "[Camera " << i << "] Image captured successfully for OffsetX: " << roi.offset_x << " (Image Index: " << circular_index << ")\n";

                        // Increment count for the current offset
                        image_counts[i][roi.offset_x]++;
                    }
                    catch (const Spinnaker::Exception& e)
                    {
                        cerr << "[Camera " << i << "] Error capturing image: " << e.what() << endl;
                        result = -1;
                    }

                    // Stop acquisition after capturing the image
                    vector<CameraPtr> acquisitioned_camera = {cameras[i]};
                    stop_camera_acquisition(acquisitioned_camera);

                    // Check for user input
                    if (keyboard_input() && handle_keyboard_interrupt())
                    {
                        cout << "User requested termination (pressed 'q'). Exiting acquisition loop.\n";
                        local_running = false;
                        global_running.store(false);
                        break;
                    }
                }

                if (!local_running)
                    break;
            }

            if (!local_running)
                break;
        }
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "Critical error during image acquisition: " << e.what() << endl;
        result = -1;
    }

    return result;
}


/**
 * Prints out the device information of the each camera from the transport layer
 * @param node_map: The GenICam node map for the camera.
 * @return 0 if successful, -1 if an error occurred during device information retrieval.
 */
int CAMERA_MANAGER::print_device_info(INodeMap& node_map)
{
    int result = 0;

    cout << "\n\n*** DEVICE INFORMATION ***.\n\n";

    try
    {
        FeatureList_t features;
        const CCategoryPtr category = node_map.GetNode("DeviceInformation");
        if (IsReadable(category))
        {
            category->GetFeatures(features);
 
            for (auto it = features.begin(); it != features.end(); ++it)
            {
                const CNodePtr pfeatureNode = *it;
                cout << pfeatureNode->GetName() << " : ";
                CValuePtr pValue = static_cast<CValuePtr>(pfeatureNode);
                cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
                cout << endl;
            }
        }
        else
        {
            cout << "Device control information not available.\n";
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }
 
    return result;
}

/**
 * Cleans up the cameras and node maps.
 * @param cameras: The camera pointers to acquire images from.
 * @param initialized_cameras: The successfully initialized camera pointers.
 * @param node_maps: The GenICam node maps for the cameras.
 * @param node_maps_tl_device: The GenICam node maps for the TL devices.
 */
void CAMERA_MANAGER::de_initialize_cameras(vector<CameraPtr>& cameras, vector<CameraPtr>& initialized_cameras, vector<INodeMap*>& node_maps, vector<INodeMap*>& node_maps_tl_device)
{
    // Stop acquisition for all cameras
    stop_camera_acquisition(initialized_cameras);

    cout << "\n\n*** DEINITIALIZING CAMERAS ***\n\n";
    
    // Deinitialize all successfully initialized cameras
    for (size_t i = 0; i < initialized_cameras.size(); ++i)
    {
        CameraPtr& camera = initialized_cameras[i];
        if (camera && camera->IsInitialized())
        {
            try
            {
                camera->DeInit();
                cout << "[Camera " << i << "] deinitialized successfully.\n";
            }
            catch (const Spinnaker::Exception& e)
            {
                cerr << "[Camera " << i << "] Error during deinitialization: " << e.what() << endl;
            }
        }
    }
    initialized_cameras.clear(); // Clear initialized cameras

    // Clear node maps
    cout << "Clearing node maps. GenICam node maps: " << node_maps.size() << ", Transport layer node maps: " << node_maps_tl_device.size() << ".\n";

    // Clear node maps
    node_maps.clear();
    node_maps_tl_device.clear();

    // Nullify all camera pointers in the main list
    for (size_t i = 0; i < cameras.size(); ++i)
    {
        if (cameras[i] != nullptr)
        {
            cameras[i] = nullptr;
            cout << "[Camera " << i << "] pointer nullified.\n";
        }
        else
        {
            cout << "[Camera " << i << "] pointer was already null.\n";
        }
    }

    cout << "Cameras and node maps cleaned up successfully.\n";
}

/**
 * Runs the camera configuration and image acquisition
 * @param cameras: The camera pointers to acquire images from.
 * @param cam_list: The camera list to select cameras from.
 * @param number_of_cameras: The number of cameras to acquire images from.
 * @param global_running: The atomic boolean to control the running state of the cameras.
 * @return 0 if successful, -1 if an error occurred during camera operations.
 */
int CAMERA_MANAGER::run_multiple_cameras(vector<CameraPtr>& cameras, CameraList& cam_list, unsigned int number_of_cameras, atomic<bool>& global_running, const string& folder_path)
{
    int result = 0;
    int is_exposure_config_ok = 0;

    vector<INodeMap*> node_maps;
    vector<INodeMap*> node_maps_tl_device;
    vector<CameraPtr> initialized_cameras;

    try
    {
        // Initialize cameras and retrieve node maps
        for (unsigned int i = 0; i < number_of_cameras; i++)
        {
            CameraPtr camera = cameras[i];
            if (!camera)
            {
                cerr << "[Camera " << i << "] is invalid. Skipping.\n";
                continue;
            }

            try
            {
                // Retrieve and validate transport layer node map
                INodeMap* node_map_tl_device = &camera->GetTLDeviceNodeMap();
                if (!node_map_tl_device)
                {
                    cerr << "[Camera " << i << "] Failed to retrieve transport layer node map. Skipping.\n";
                    continue;
                }
                if (print_device_info(*node_map_tl_device) != 0)
                {
                    cerr << "[Camera " << i << "] Device info retrieval failed. Skipping.\n";
                    continue;
                }
                node_maps_tl_device.push_back(node_map_tl_device);

                // Initialize camera
                camera->Init();
                initialized_cameras.push_back(camera);

                // Retrieve GenICam node map
                INodeMap* node_map = &camera->GetNodeMap();
                if (!node_map)
                {
                    cerr << "[Camera " << i << "] Failed to retrieve GenICam node map. Deinitializing camera.\n";
                    camera->DeInit();
                    continue;
                }
                node_maps.push_back(node_map);

                cout << "[Camera " << i << "] Initialized successfully.\n";
            }
            catch (const Spinnaker::Exception& e)
            {
                cerr << "[Camera " << i << "] Initialization error: " << e.what() << endl;
                continue;
            }
        }

        if (initialized_cameras.size() < number_of_cameras)
        {
            cerr << "Not all cameras were successfully initialized. Terminating.\n";

            // Clean up initialized cameras and node maps
            de_initialize_cameras(cameras, initialized_cameras, node_maps, node_maps_tl_device);

            return -1;
        }

        cout << "\n*** " << initialized_cameras.size() << " CAMERAS SUCCESSFULLY INITIALIZED ***\n";

        // Run camera configurations
        cout << "Running camera configurations...\n";
        result |= config_pixel_format(node_maps);
        result |= config_sensor_shutter_mode(node_maps);

        is_exposure_config_ok = config_exposure(node_maps);
        if (is_exposure_config_ok < 0)
        {
            cerr << "Exposure configuration failed. Terminating.\n";
            return is_exposure_config_ok;
        }

        result |= config_gain(node_maps);
        result |= config_black_level_clamping_enable(node_maps);
        result |= config_gamma(node_maps);

        // Run image acquisition
        result |= acquire_images(initialized_cameras, initialized_cameras.size(), node_maps, node_maps_tl_device, global_running, folder_path);

        if (is_exposure_config_ok == 0)
        {
            result |= reset_exposure(node_maps);
        }
        else
        {
            cerr << "Error during image acquisition. Terminating.\n";
        }
    }
    catch (const Spinnaker::Exception& e)
    {
        cerr << "Critical error during camera operations: " << e.what() << endl;
        result = -1;
    }

    // Clean up resources
    de_initialize_cameras(cameras, initialized_cameras, node_maps, node_maps_tl_device);

    return result;
}
