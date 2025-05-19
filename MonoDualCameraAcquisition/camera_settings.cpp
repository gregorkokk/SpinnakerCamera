// Description: Get camera setting values from a file and store them in a struct and provide getters for the values.
// Author: Gregor Kokk
// Date: 06.01.2025

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#include "camera_settings.h"

// Load the content of a file into a vector of strings
std::vector<std::string> CAMERA_SETTINGS::load_from_file(const std::string &filename)
{
    std::vector<std::string> file_content;
    std::ifstream file_in(filename);

    if (!file_in)
    {
        std::cerr << "Error opening file: " << filename << '\n';
        return {};
    }

    std::string line;
    while (std::getline(file_in, line))
    {
        file_content.push_back(line);
    }

    return file_content;
}

// Parse a single line into a key and value
std::pair<std::string, double> CAMERA_SETTINGS::parse_line(const std::string &line)
{
    std::stringstream ss(line);
    std::string key;
    double value;

    if (std::getline(ss, key, ':') && ss >> value)
    {
        return {key, value};
    }

    std::cerr << "Error parsing line: " << line << '\n';
    return {"", -1.0};
}

// Parse file content and assign values to camera settings
int CAMERA_SETTINGS::get_values(const std::vector<std::string> &file_content)
{
    std::cout << "\n*** GET VALUES ***\n\n";

    for (const auto &line : file_content)
    {
        std::pair<std::string, double> parsed_line = parse_line(line); // No structured bindings
        const std::string& key = parsed_line.first; // Access key
        double value = parsed_line.second;          // Access value

        if (key == "Exposure")
        {
            settings.exposure = value;
            std::cout << "Exposure: " << value << "\n";
        }
        else if (key == "Gain")
        {
            settings.gain = value;
            std::cout << "Gain: " << value << "\n";
        }
        else if (key == "Gamma")
        {
            settings.gamma = value;
            std::cout << "Gamma: " << value << "\n";
        }
        else
        {
            std::cerr << "Unknown key: " << key << '\n';
        }
    }

    return 0;
}

// Getter for Exposure
double CAMERA_SETTINGS::get_exposure() const
{
    return settings.exposure;
}

// Getter for Gain
double CAMERA_SETTINGS::get_gain() const
{
    return settings.gain;
}

// Getter for Gamma
double CAMERA_SETTINGS::get_gamma() const
{
    return settings.gamma;
}