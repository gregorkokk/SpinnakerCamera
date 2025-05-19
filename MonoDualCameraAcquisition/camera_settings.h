// camera_settings.cpp Header File
// Author: Gregor Kokk
// Date: 06.01.2025

#ifndef CAMERA_SETTINGS_H
#define CAMERA_SETTINGS_H

#include <string>
#include <vector>

using namespace std;

class CAMERA_SETTINGS
{
private:
    struct SETTINGS // Struct to hold camera settings
    {
        double exposure;
        double gain;
        double gamma;
    };

    SETTINGS settings;   // Instance of settings struct

    // Helper function to parse a line into a key-value pair
    pair<string, double> parse_line(const string &line);

public:
    // Function to load the content of a file into a vector of strings
    vector<string> load_from_file(const string &filename);
    
    // Function to extract values from the file content
    int get_values(const vector<string> &file_content);

    // Getters for settings
    double get_exposure() const;
    double get_gain() const;
    double get_gamma() const;
};

#endif // CAMERA_SETTINGS_H