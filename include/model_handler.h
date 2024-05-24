#ifndef MODEL_HANDLER_H
#define MODEL_HANDLER_H

#include <string>
#include <vector>
#include <map>
#include <boost/filesystem.hpp>
#include <json/json.h>

class ModelHandler {
public:
    ModelHandler(const boost::filesystem::path& json_file_path);

    std::vector<std::pair<int, double>> getHarmonicDriveValues(const std::string& prefix = "B");
    void setHarmonicDriveValue(const std::string& name, double value);

private:
    void createTemporaryFolder(const boost::filesystem::path& json_file_path);
    void parseHarmonicDrive(const Json::Value& root, const std::string& prefix, std::map<int, double>& harmonics_map);
    void updateHarmonicDrive(Json::Value& root, const std::string& name, double value);

    boost::filesystem::path temp_folder_;
    boost::filesystem::path temp_json_path_;
};

#endif // MODEL_HANDLER_H
