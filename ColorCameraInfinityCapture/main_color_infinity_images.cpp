// BFS-U3-50S5C-C (Blackfly S) camera configuration and capturing of pictures till I stop the program
// Author: Gregor Kokk
// Date: 2024

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <functional>
#include <chrono>	// for std::chrono::milliseconds
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>	// for std::this_thread::sleep_for
#include <csignal>	// for signal handling


#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include "main.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

// Function to load the content of a file into a vector of strings
vector<string> CAMERA_CONFIG::load_from_file(const string &filename)
{
    vector<string> file_content;

    try
    {
        ifstream file_in(filename);
        string line;

        while (getline(file_in, line))
        {
            file_content.push_back(line);
        }
        return file_content;
    }
    catch (const exception &e)
    {
        cerr << "Error opening file: " << filename << e.what() << '\n';
        return {};
    }
}

// Function to extract value from a line (private)
double CAMERA_CONFIG::extract_value_from_line(const string &line)
{
    size_t colon_position = line.find(':');

    if (colon_position != string::npos)
    {
        string value_string = line.substr(colon_position + 1);
        stringstream ss(value_string);
        double value;
        ss >> value;

        return value;
    }
    return -1.0; // Return -1 if the format is wrong
}

// Function to extract values from the file content and store them in the settings (public)
void CAMERA_CONFIG::get_values(const vector<string>& file_content)
{
    for (const auto &line : file_content)
    {
        if (line.find("Exposure") != string::npos)
        {
            settings.exposure = extract_value_from_line(line);
        }
        else if (line.find("Gain") != string::npos)
        {
            settings.gain = extract_value_from_line(line);
        }
        else if (line.find("Sharpening") != string::npos)
        {
            settings.sharpening = extract_value_from_line(line);
        }
        else if (line.find("Gamma") != string::npos)
        {
            settings.gamma = extract_value_from_line(line);
        }
        else if (line.find("Saturation") != string::npos)
        {
            settings.saturation = extract_value_from_line(line);
        }
    }
}

// This function controls saturation
int CAMERA_CONFIG::config_saturation(INodeMap& node_map)
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
                cout << "Unable to get or set saturation. Aborting" << endl;
                return -1;
            }
            
            // Check if the saturation value is within the acceptable range
            if (settings.saturation > ptr_saturation->GetMax())
            {
                settings.saturation = ptr_saturation->GetMax();
                cout << "Saturation value too high. Set to maximum value" << endl;
            }
            else if (settings.saturation < ptr_saturation->GetMin())
            {
                settings.saturation = ptr_saturation->GetMin();
                cout << "Saturation value too low. Set to minimum value" << endl;
            }

            ptr_saturation->SetValue(settings.saturation);
            cout << "Saturation set to: " << ptr_saturation->GetValue() << endl;
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

// This function controls gain
int CAMERA_CONFIG::config_gain(INodeMap& node_map)
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
        }

        CFloatPtr ptr_gain = node_map.GetNode("Gain"); // Set gain manually, in dB
        if (!IsReadable(ptr_gain) || !IsWritable(ptr_gain))
        {
            cout << "Unable to get or set gain. Aborting" << endl;
            return -1;
        }

        // Check if the gain value is within the acceptable range
        if (settings.gain > ptr_gain->GetMax())
        {
            settings.gain = ptr_gain->GetMax();
            cout << "Gain value too high. Set to maximum value" << endl;
        }
        else if (settings.gain < ptr_gain->GetMin())
        {
            settings.gain = ptr_gain->GetMin();
            cout << "Gain value too low. Set to minimum value" << endl;
        }

        ptr_gain->SetValue(settings.gain);
        cout << "Gain set to " << ptr_gain->GetValue() << endl;

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function controls gamma
int CAMERA_CONFIG::config_gamma(INodeMap& node_map)
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
                cout << "Unable to get or set gamma. Aborting" << endl;
                return -1;
            }

            // Check if the gamma value is within the acceptable range
            if (settings.gamma > ptr_gamma->GetMax())
            {
                settings.gamma = ptr_gamma->GetMax();
                cout << "Gamma value too high. Set to maximum value" << endl;
            }
            else if (settings.gamma < ptr_gamma->GetMin())
            {
                settings.gamma = ptr_gamma->GetMin();
                cout << "Gamma value too low. Set to minimum value" << endl;
            }

            ptr_gamma->SetValue(settings.gamma);
            cout << "Gamma set to: " << ptr_gamma->GetValue() << endl;
        }
        else
        {
            cout << "Unable to enable gamma" << endl;
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
int CAMERA_CONFIG::config_sharpening(INodeMap& node_map)
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
                cout << "Unable to get or set sharpening. Aborting" << endl;
                return -1;
            }

            // Check if the sharpness value is within the acceptable range
            if (settings.sharpening > ptr_sharpening->GetMax())
            {
                settings.sharpening = ptr_sharpening->GetMax();
                cout << "Sharpening value too high. Set to maximum value" << endl;
            }
            else if (settings.sharpening < ptr_sharpening->GetMin())
            {
                settings.sharpening = ptr_sharpening->GetMin();
                cout << "Sharpening value too low. Set to minimum value" << endl;
            }

            ptr_sharpening->SetValue(settings.sharpening);
            cout << "Sharpness set to: " << ptr_sharpening->GetValue() << endl;
        }
        else
        {
            cout << "Unable to enable sharpening" << endl;
        }

    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function configures a custom exposure time.
int CAMERA_CONFIG::config_exposure(INodeMap& node_map)
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
        
        // Check if the exposure value is within the acceptable range
        if (settings.exposure > ptr_exposure_time->GetMax())
        {
            settings.exposure = ptr_exposure_time->GetMax();
            cout << "Exposure value too high. Set to maximum value" << endl;
        }
        else if (settings.exposure < ptr_exposure_time->GetMin())
        {
            settings.exposure = ptr_exposure_time->GetMin();
            cout << "Exposure value too low. Set to minimum value" << endl;
        }
        
        ptr_exposure_time->SetValue(settings.exposure);
        cout << std::fixed << "Exposure time set to: " << ptr_exposure_time->GetValue() << " μs" << endl;
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
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function configures the camera to use a custom region of interest (ROI) -> width, height
int CAMERA_CONFIG::config_roi(INodeMap& node_map, int64_t width_value, int64_t height_value)
{
    int result = 0;

    cout << endl << endl << "*** CONFIGURING ROI: HEIGHT, WIDTH ***" << endl << endl;

    try
    {
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
        }
    }
    catch(Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

// This function prints out the device information of the camera from the transport layer
int CAMERA_CONFIG::print_device_info(INodeMap& node_map)
{
    int result = 0;

    cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

    try
    {
        FeatureList_t features;
        CCategoryPtr category = node_map.GetNode("DeviceInformation");
        if (IsReadable(category))
        {
            category->GetFeatures(features);

            FeatureList_t::const_iterator it;
            for (it = features.begin(); it != features.end(); ++it)
            {
                CNodePtr feature_node = *it;
                cout << feature_node->GetName() << " : ";
                CValuePtr ptr_value = (CValuePtr)feature_node;
                cout << (IsReadable(ptr_value) ? ptr_value->ToString() : "Node not readable");
                cout << endl;
            }
        }
        else
        {
            cout << "Device control information not readable" << endl;
        }
    }
    catch (Spinnaker::Exception& e)
    {
        cout << "Error: " << e.what() << endl;
        result = -1;
    }

    return result;
}

void CAMERA_CONFIG::set_non_blocking_input(bool enable) // Function to set terminal input mode (non-blocking)
{
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);

    if (enable) {
        ttystate.c_lflag &= ~ICANON; // Disable canonical mode
        ttystate.c_lflag &= ~ECHO;   // Disable echo
        ttystate.c_cc[VMIN] = 1;
    } else {
        ttystate.c_lflag |= ICANON;  // Enable canonical mode
        ttystate.c_lflag |= ECHO;    // Enable echo
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int CAMERA_CONFIG::keyboard_input() // Function to get keyboard input
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

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

// This function acquires and saves images from the camera
int CAMERA_CONFIG::acquire_images(CameraPtr pointer_cam, INodeMap& node_map, INodeMap& node_map_tl_device)
{
    CAMERA_CONFIG camera_config; // Create an instance of class CAMERA_CONFIG

    camera_config.set_non_blocking_input(true); // Enable non-blocking mode

    int result = 0;
    int image_count = 0; // Image count of how many images have been acquired
    bool running = true; // Running state of the camera


    cout << endl << "*** IMAGE ACQUISITION ***" << endl << endl;

    try
    {
        CEnumerationPtr ptr_acquisition_mode = node_map.GetNode("AcquisitionMode");  // Setting acquisition mode to continuous
        if(!IsReadable(ptr_acquisition_mode) || !IsWritable(ptr_acquisition_mode))
        {
            cout << "Unable to get or set acquisition mode to continuous (node retrieval). Aborting." << endl;
            return -1;
        }

        CEnumEntryPtr ptr_acquisition_mode_continuous = ptr_acquisition_mode->GetEntryByName("Continuous");
        if (!IsReadable(ptr_acquisition_mode_continuous))
        {
            cout << "Unable to get acquisition mode to continuous (entry 'continuous' retrieval). Aborting..." << endl;
            return -1;
        }

        const int64_t acquisition_mode_continuous = ptr_acquisition_mode_continuous->GetValue();

        ptr_acquisition_mode->SetIntValue(acquisition_mode_continuous);

        cout << "Acquisition mode set to continuous" << endl;

        //Begin acquiring images
        pointer_cam->BeginAcquisition();

        cout << "Acquiring images" << endl;

        CFloatPtr ptr_exposure_time = node_map.GetNode("ExposureTime"); // Get the value of exposure time to set an appropriate timeout for GetNextImage
        if(!IsReadable(ptr_exposure_time))
        {
            cout << "Unable to get or set exposure time. Aborting" << endl;
            return -1;
        }

        uint64_t timeout = static_cast<uint64_t>(ptr_exposure_time->GetValue() / 1000 + 1000);

        ImageProcessor processor;   // Create image processor instance for post processing images

        processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_DIRECTIONAL_FILTER);  // Set interpolation algorithm

        auto start_time_image = chrono::steady_clock::now(); // Start the time for image  data

        while(running)  // Continue recording until the user stops it
        {
            auto start_time = chrono::steady_clock::now(); // Start time for image acquisition
            try
            {
                // Retrive next received image and ensure image completion
                // Timeout value is set to [exposure time + 1000] ms to ensure that the image has enough time to arrive
                ImagePtr p_result_image_pointer = pointer_cam->GetNextImage(timeout);

                if (p_result_image_pointer->IsIncomplete())
                {
                    cout << "Image incomplete with image status " << p_result_image_pointer->GetImageStatus() << endl << endl;
                }
                else
                {
                    // Convert image to custom color processing algorithm
                    ImagePtr converted_image = processor.Convert(p_result_image_pointer, PixelFormat_BGR8); // Convert to BGR format
                    
                    auto current_time_image = chrono::steady_clock::now(); // Current time for image data
                    auto elapsed_time_image = chrono::duration_cast<chrono::seconds>(current_time_image - start_time_image); // Calculate elapsed time for image data
                    
                    int minutes = elapsed_time_image.count() / 60; // Calculate minutes
                    int seconds = elapsed_time_image.count() % 60; // Calculate seconds

                    size_t width = p_result_image_pointer->GetWidth();
                    size_t height = p_result_image_pointer->GetHeight();

                    cout << "Grabbed image " << image_count << ", width = " << width << ", height = " << height << endl;

                    // Define the folder path to save images
                    string folder_path = "/folder/path/to/save/images"; // Folder path to save images
                    
                    ostringstream filename; // Create a unique filename

                    filename << folder_path << "image_" << image_count + 1 << "_"<< minutes << ":" << seconds << ".jpg";; // Prefix with folder path and image count

                    converted_image->Save(filename.str().c_str());

                    cout << "Image saved at " << filename.str() << endl;

                    image_count++; // Increment image count

                    if (camera_config.keyboard_input()) // Check if the user has pressed a key
                    {
                        char key = getchar();
                        if(key == 'q' || key == 'Q') // If the user presses 'q', exit the loop
                        {
                            running = false;
                        }
                    }
                }
                p_result_image_pointer->Release();  // Release image

            }
            catch (Spinnaker::Exception& e)
            {
                cout << "Error: " << e.what() << endl;
                result = -1;
            }

            auto end_time = chrono::steady_clock::now(); // End time for image acquisition
            chrono::duration<double> elapsed_seconds = end_time - start_time; // Calculate elapsed time
            int delay_time = (1000 - static_cast<int>(elapsed_seconds.count() * 1000)) / 2; // Calculate delay time

            cout << "Elapsed time: " << elapsed_seconds.count() << " seconds" << endl;
            cout << "Delay time: " << delay_time << " milliseconds" << endl;

            if (delay_time > 0)
            {
                this_thread::sleep_for(chrono::milliseconds(delay_time)); // Wait for the remaining time
            }
        }
        pointer_cam->EndAcquisition();  // End acquisition
        camera_config.set_non_blocking_input(false);   // Set input to blocking mode
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
        INodeMap& node_map_tl_device = pointer_cam->GetTLDeviceNodeMap();   // Retrieve TL device nodemap and print device information

        cout << "Initialize camera \n" << endl;
        pointer_cam->Init();    // Initialize camera

        INodeMap& node_map = pointer_cam->GetNodeMap();  // Retrieve GenICam nodemap

        cout << "Running print device info function" << endl;
        result = result | CAMERA_CONFIG::print_device_info(node_map_tl_device);          // Calling out print_defice_info function and checking if it returns 0

        cout << "Running pixel format function" << endl;
        result = result | CAMERA_CONFIG::config_pixel_format(node_map); // Pixel Format

        cout << "Running camera settings" << endl;
        result = result | CAMERA_CONFIG::config_roi(node_map, 1424, 408); // Width, Height[pixels]
        result = result | CAMERA_CONFIG::config_exposure(node_map); // Exposure 33.0 [μs] to 30.0 [s]
        result = result | CAMERA_CONFIG::config_gain(node_map); // Gain 0.0 to 47.9943 [dB])
        result = result | CAMERA_CONFIG::config_sharpening(node_map); // Sharpness -1 to 8
        result = result | CAMERA_CONFIG::config_gamma(node_map); // Gamma, (0.1 to 4.0)
        result = result | CAMERA_CONFIG::config_saturation(node_map); // Saturation 0.0 to 1.0

        cout << "Running acquire images function \n" << endl;
        result = result | CAMERA_CONFIG::acquire_images(pointer_cam, node_map, node_map_tl_device); // Calling out acquire_images function and checking if it returns 0   
        
        if (result == 0)
        {
            cout << "Running reset exposure function" << endl;
            result = result | CAMERA_CONFIG::reset_exposure(node_map);  // Calling out reset_exposure function and checking if it returns 0
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

// Main function
int main(int argc, char** argv)
{
    int result = 0;

    SystemPtr system = System::GetInstance(); // Retrieve singleton reference to system object

    CameraList camera_list = system->GetCameras(); // Retrieve list of cameras from the system

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

    // Load file content
    vector<string> file_content = camera_config.load_from_file("/path/to/the/database_color.txt");

    // Extract values from the file content
    camera_config.get_values(file_content);

    for (unsigned int i = 0; i < num_cameras; i++)  // Run configuration on each camera
    {
        cout << "Running configuration for camera " << i << "..." << endl;

        result = result | camera_config.run_single_camera(camera_list.GetByIndex(i));

        cout << "Camera " << i << " configuration complete" << endl;
    }

    camera_list.Clear();    // Release camera list before releasing system

    system->ReleaseInstance();  // Release system

    cout << "Done! Press Enter to exit" << endl;
    getchar();

    return result;
}
