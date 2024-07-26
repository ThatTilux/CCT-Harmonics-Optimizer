#ifndef MODEL_HANDLER_H
#define MODEL_HANDLER_H

#include <string>
#include <vector>
#include <regex>
#include <map>
#include <boost/filesystem.hpp>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <stdexcept>



// data type for parameters of custom cct harmonics with 'amplitude' = linear | constant
enum class HarmonicDriveParameterType {
    Offset,
    Slope,
    Constant
};

// Forward declaration of HarmonicDriveParameters
struct HarmonicDriveParameters;

// Typedefinition for the unordered map
// Keys have format "B1" to "B10"
using HarmonicDriveParameterMap = std::unordered_map<std::string, HarmonicDriveParameters>;

class ModelHandler {
public:
    ModelHandler(const boost::filesystem::path& json_file_path);
    ModelHandler(); // dummy - do not use

    HarmonicDriveParameterMap getHarmonicDriveValues(const std::string& prefix = "B");
    void setHarmonicDriveValue(const std::string &name, const HarmonicDriveParameters &params);
    void apply_params(const HarmonicDriveParameterMap &paramMap);
    boost::filesystem::path getTempJsonPath() const;

private:
    void createTemporaryFolder(const boost::filesystem::path& json_file_path);
    void parseHarmonicDrive(const Json::Value &root, HarmonicDriveParameterMap &harmonics_map, const std::string &prefix = "B", const std::regex &suffix_regex = std::regex(R"(\d{1,2})")); // suffix means 1 or 2 numbers
    void updateHarmonicDrive(Json::Value &root, const std::string &name, const HarmonicDriveParameterType &type, double value);


    boost::filesystem::path temp_folder_;
    boost::filesystem::path temp_json_path_;
};

#endif // MODEL_HANDLER_H
