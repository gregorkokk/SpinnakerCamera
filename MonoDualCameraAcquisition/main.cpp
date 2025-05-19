// Description: This is the main file for the getting values from database & two-camera acquisition
// Author: Gregor Kokk
// Date: 06.01.2025

#include <iostream>
#include <string>
#include <vector>
#include <chrono>	// For std::chrono::milliseconds
#include <thread>	// For std::this_thread::sleep_for
#include <atomic>   // For std::atomic --> To communicate between the acquire_images function and the main function

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"

#include "camera_manager.h"
#include "camera_settings.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;

int main(int argc, char** argv)
{
    // Print application build information
    cout << "Application build date: " << __DATE__ << " " << __TIME__ << endl << endl;

    const int max_retries = 3; // Maximum retries for initialization
    int retries = 0;
    atomic<bool> global_running(true); // Indicates if the program should continue running

    string folder_path = "/path/to/save/images";	// Folder path to save images

    while (retries < max_retries)   // Retry initialization both cameras don't get detected, or if an error occurs
    {
        // Retrieve singleton reference to system object
        SystemPtr system = System::GetInstance();
        if (!system)
        {
            cerr << "Failed to retrieve Spinnaker system instance.\n";
            return -1;
        }

        // Retrieve list of cameras from the system
        CameraList camera_list = system->GetCameras();
        unsigned int number_of_cameras = camera_list.GetSize();

        cout << "Number of cameras detected: " << number_of_cameras << "\n";

        // If no cameras detected, retry after a delay
        if (number_of_cameras < 2)
        {
            cerr << "Less than two cameras detected. Retrying... (" << retries + 1 << "/" << max_retries << ")" << endl;
            camera_list.Clear(); // Release camera list before releasing system
            system->ReleaseInstance();
            retries++;
            this_thread::sleep_for(chrono::seconds(2)); // Add a delay between retries
            continue;
        }

        // Create a vector to hold CameraPtr objects
        vector<CameraPtr> cameras;
        for (unsigned int i = 0; i < number_of_cameras; i++)
        {
            cameras.push_back(camera_list.GetByIndex(i));
        }

        int result = 0;

        try
        {
            // Initialize CAMERA_SETTINGS & CAMERA_MANAGER
            CAMERA_SETTINGS camera_settings;
            CAMERA_MANAGER camera_manager(&camera_settings); // Pass pointer to camera settings object

            // Load camera configuration from file
            vector<string> file_content = camera_settings.load_from_file("/path/to/database/mono.txt");
            if (file_content.empty())
            {
                cerr << "Failed to load camera configuration from file. Exiting.\n";
                return -1;
            }

            result |= camera_settings.get_values(file_content); // Get values from file content
            cout << "Camera configuration loaded successfully.\n";

            // Run configuration and image acquisition on multiple cameras
            result |= camera_manager.run_multiple_cameras(cameras, camera_list, number_of_cameras, global_running, folder_path);

            if (result == 0)
            {
                cout << "All cameras configured and operated successfully.\n";
            }
        }
        catch (const Spinnaker::Exception& e)
        {
            cerr << "Error during camera configuration or operation: " << e.what() << endl;
            result = -1;
        }

        // Explicitly release references to each camera
        for (auto& camera : cameras)
        {
            camera = nullptr; // Break the reference to the camera
        }

        // Release camera list before releasing system
        camera_list.Clear();

        // Release system instance
        system->ReleaseInstance();

        // Check termination conditions
        if (result == 0 || !global_running)
        {
            if (!global_running)
            {
                cout << "Program terminated by user.\n";
            }
            cout << "Exiting...\n";
            break;
        }

        cerr << "Retrying due to errors... (" << retries + 1 << "/" << max_retries << ")" << endl;
        retries++;
        this_thread::sleep_for(chrono::seconds(2)); // Delay before retry
    }

    // Check if retries were exhausted
    if (retries == max_retries)
    {
        cerr << "Failed to configure cameras after " << max_retries << " retries. Exiting.\n";
        return -1;
    }

    // Final message
    cout << endl << "Done! Press Enter to exit..." << endl;
    getchar();

    return 0;
}