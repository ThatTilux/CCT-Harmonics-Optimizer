#include "model_handler.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

// Constructor
ModelHandler::ModelHandler(const boost::filesystem::path& json_file_path) {
    createTemporaryFolder(json_file_path);
}

// Creates a tempoary folder with a copy of the model. All operations are done on that model
void ModelHandler::createTemporaryFolder(const boost::filesystem::path& json_file_path) {
    temp_folder_ = boost::filesystem::temp_directory_path() / "model_temp";
    if (boost::filesystem::exists(temp_folder_)) {
        boost::filesystem::remove_all(temp_folder_);
    }
    boost::filesystem::create_directory(temp_folder_);
    temp_json_path_ = temp_folder_ / json_file_path.filename();
    boost::filesystem::copy_file(json_file_path, temp_json_path_);
}

// Parses the json file and extracts all the scaling function constants/slopes for all the custom cct harmonics
// Throws an error if there are multiple harmonics with the same name but different constants/slopes
std::vector<std::pair<int, double>> ModelHandler::getHarmonicDriveValues(const std::string& prefix) {
    std::ifstream file(temp_json_path_.string());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file.");
    }

    Json::Value root;
    file >> root;
    file.close();

    std::map<int, double> harmonics_map;
    parseHarmonicDrive(root, prefix, harmonics_map);

    std::vector<std::pair<int, double>> result(harmonics_map.begin(), harmonics_map.end());
    return result;
}

// Updates all the custom cct harmonics' scaling constants/slopes for the given name
void ModelHandler::setHarmonicDriveValue(const std::string& name, double value) {
    std::ifstream file(temp_json_path_.string());
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open JSON file.");
    }

    Json::Value root;
    file >> root;
    file.close();

    updateHarmonicDrive(root, name, value);

    std::ofstream out_file(temp_json_path_.string());
    out_file << root;
    out_file.close();
}

// Recursive helper function to get all the scaling function constants/slopes for all the custom cct harmonics
void ModelHandler::parseHarmonicDrive(const Json::Value& root, const std::string& prefix, std::map<int, double>& harmonics_map) {
    if (root.isObject()) {
        if (root["type"].asString() == "rat::mdl::cctharmonicdrive" && root["name"].asString().substr(0, prefix.size()) == prefix) {
            int num = std::stoi(root["name"].asString().substr(prefix.size()));
            double value;

            if (root["harmonic_drive"]["type"].asString() == "rat::mdl::drivedc") {
                value = root["harmonic_drive"]["scaling"].asDouble();
            } else if (root["harmonic_drive"]["type"].asString() == "rat::mdl::drivelinear") {
                value = root["harmonic_drive"]["slope"].asDouble();
            } else {
                throw std::runtime_error("Unsupported harmonic drive type: " + root["harmonic_drive"]["type"].asString());
            }

            if (harmonics_map.find(num) != harmonics_map.end() && harmonics_map[num] != value) {
                throw std::runtime_error("Conflicting values for harmonic drive with the name B" + std::to_string(num) + 
                                         ". Values: " + std::to_string(harmonics_map[num]) + " and " + std::to_string(value));
            }

            harmonics_map[num] = value;
        }
    }

    for (const auto& element : root) {
        parseHarmonicDrive(element, prefix, harmonics_map);
    }
}

// Recursive helper function to update all the custom cct harmonics' scaling constants/slopes for the given name
void ModelHandler::updateHarmonicDrive(Json::Value& root, const std::string& name, double value) {
    if (root.isObject()) {
        if (root["type"].asString() == "rat::mdl::cctharmonicdrive" && root["name"].asString() == name) {
            if (root["harmonic_drive"]["type"].asString() == "rat::mdl::drivedc") {
                root["harmonic_drive"]["scaling"] = value;
            } else if (root["harmonic_drive"]["type"].asString() == "rat::mdl::drivelinear") {
                root["harmonic_drive"]["slope"] = value;
            }
        }
    }

    for (Json::Value& element : root) {
        updateHarmonicDrive(element, name, value);
    }
}
