#include "gtest/gtest.h"
#include "harmonic_drive_parameter.h"
#include "model_handler.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <constants.h>

// Test fixture for ModelHandler tests
class ModelHandlerTest : public ::testing::Test
{
protected:
    boost::filesystem::path test_file;
    boost::filesystem::path test_file_2;
    boost::filesystem::path temp_dir;

    // Setup before each test
    void SetUp() override
    {
        test_file = TEST_DATA_DIR + "quad_test.json";
        test_file_2 = TEST_DATA_DIR + "quad_test_B5_linear.json";
        temp_dir = boost::filesystem::temp_directory_path() / "model_temp";

        // Ensure the temp directory is clean
        if (boost::filesystem::exists(temp_dir))
        {
            boost::filesystem::remove_all(temp_dir);
        }
    }

    // Cleanup after each test
    void TearDown() override
    {
        if (boost::filesystem::exists(temp_dir))
        {
            boost::filesystem::remove_all(temp_dir);
        }
    }
};

// Test the constructor and createTemporaryFolder method
TEST_F(ModelHandlerTest, ConstructorAndCreateTemporaryFolder)
{
    boost::filesystem::path temp_json_path;

    ASSERT_NO_THROW({
        CCTools::ModelHandler handler(test_file);
        temp_json_path = handler.getTempJsonPath();
    });

    // Check if the temp directory and JSON file were created
    EXPECT_TRUE(boost::filesystem::exists(temp_dir));
    EXPECT_TRUE(boost::filesystem::exists(temp_json_path));
}

bool containsParameterValue(CCTools::HarmonicDriveParameterMap map, std::string name, CCTools::HarmonicDriveParameterType type, double value, double margin)
{
    for (const auto &pair : map)
    {
        if (pair.first == name && std::abs(pair.second.get(type) - value) <= margin)
        {
            return true;
        }
    }
    return false;
}

// Test the getHarmonicDriveValues method
TEST_F(ModelHandlerTest, GetHarmonicDriveValues)
{
    CCTools::ModelHandler handler(test_file);
    CCTools::HarmonicDriveParameterMap harmonic_drive_values;

    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    // Verify the parsed values
    EXPECT_EQ(harmonic_drive_values.size(), 9);

    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B1", CCTools::HarmonicDriveParameterType::Constant, 3.0274872794616347e-05, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B3", CCTools::HarmonicDriveParameterType::Constant, -0.00018617604979581347, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B4", CCTools::HarmonicDriveParameterType::Constant, -0.00024645416164351607, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B5", CCTools::HarmonicDriveParameterType::Constant, -0.00020169498553400584, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B6", CCTools::HarmonicDriveParameterType::Constant, -0.001462563623493985, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B7", CCTools::HarmonicDriveParameterType::Constant, 0, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B8", CCTools::HarmonicDriveParameterType::Constant, 0, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B9", CCTools::HarmonicDriveParameterType::Constant, 0, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B10", CCTools::HarmonicDriveParameterType::Constant, 0, 1e-6));
}

// Test the setHarmonicDriveValue method (aplitude = constant)
TEST_F(ModelHandlerTest, SetHarmonicDriveValueConstant)
{
    CCTools::ModelHandler handler(test_file);
    double new_value = 1.23456789;

    ASSERT_NO_THROW({
        handler.setHarmonicDriveValue("B1", CCTools::HarmonicDriveParameters(new_value, CCTools::HarmonicDriveParameterType::Constant));
    });

    // Verify the updated value
    CCTools::HarmonicDriveParameterMap harmonic_drive_values;
    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B1", CCTools::HarmonicDriveParameterType::Constant, 1.23456789, 1e-6));
}

// Test the setHarmonicDriveValue method (aplitude = linear)
TEST_F(ModelHandlerTest, SetHarmonicDriveValueLinear)
{
    CCTools::ModelHandler handler(test_file_2);
    double new_slope = 1.23456789;
    double new_offset = 2.23456789;

    ASSERT_NO_THROW({
        handler.setHarmonicDriveValue("B5", CCTools::HarmonicDriveParameters(new_slope, CCTools::HarmonicDriveParameterType::Slope));
        handler.setHarmonicDriveValue("B5", CCTools::HarmonicDriveParameters(new_offset, CCTools::HarmonicDriveParameterType::Offset));
    });

    // Verify the updated value
    CCTools::HarmonicDriveParameterMap harmonic_drive_values;
    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B5", CCTools::HarmonicDriveParameterType::Slope, 1.23456789, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B5", CCTools::HarmonicDriveParameterType::Offset, 2.23456789, 1e-6));
}

// Test to ensure that no files in test_data directory are modified after tests
TEST_F(ModelHandlerTest, NoModificationOfOriginalFiles)
{
    // Get file size and modification time of the original JSON file
    auto original_size = boost::filesystem::file_size(test_file);
    auto original_time = boost::filesystem::last_write_time(test_file);

    // Perform operations that modify the temporary file
    CCTools::ModelHandler handler(test_file);
    double new_value = 1.23456789;
    handler.setHarmonicDriveValue("quad", CCTools::HarmonicDriveParameters(new_value, CCTools::HarmonicDriveParameterType::Constant));

    // Check file size and modification time of the original JSON file again
    auto final_size = boost::filesystem::file_size(test_file);
    auto final_time = boost::filesystem::last_write_time(test_file);

    // Ensure that the original file was not modified
    EXPECT_EQ(original_size, final_size);
    EXPECT_EQ(original_time, final_time);
}

// Test for apply_params method
TEST_F(ModelHandlerTest, ApplyParams)
{
    CCTools::ModelHandler handler(test_file_2);

    // create a map with new params
    CCTools::HarmonicDriveParameterMap params;
    params["B1"] = CCTools::HarmonicDriveParameters(1.23456789, CCTools::HarmonicDriveParameterType::Constant);
    params["B3"] = CCTools::HarmonicDriveParameters(3.23456789, CCTools::HarmonicDriveParameterType::Constant);
    params["B4"] = CCTools::HarmonicDriveParameters(4.23456789, CCTools::HarmonicDriveParameterType::Constant);

    params["B5"] = CCTools::HarmonicDriveParameters(5.23456789, 6.23456789);

    params["B6"] = CCTools::HarmonicDriveParameters(7.23456789, CCTools::HarmonicDriveParameterType::Constant);
    params["B7"] = CCTools::HarmonicDriveParameters(8.23456789, CCTools::HarmonicDriveParameterType::Constant);
    params["B8"] = CCTools::HarmonicDriveParameters(9.23456789, CCTools::HarmonicDriveParameterType::Constant);
    params["B9"] = CCTools::HarmonicDriveParameters(10.23456789, CCTools::HarmonicDriveParameterType::Constant);
    params["B10"] = CCTools::HarmonicDriveParameters(11.23456789, CCTools::HarmonicDriveParameterType::Constant);

    handler.apply_params(params);

    CCTools::HarmonicDriveParameterMap new_drive_values = handler.getHarmonicDriveValues();

    // Assert that every element of  params is also in the new drive values (with the same key)
    for (const auto &pair : params)
    {
        ASSERT_TRUE(new_drive_values.count(pair.first) > 0);
        ASSERT_TRUE(pair.second == new_drive_values[pair.first]);
    }

    // and vice versa
    for (const auto &pair : new_drive_values)
    {
        ASSERT_TRUE(params.count(pair.first) > 0);
        ASSERT_TRUE(pair.second == params[pair.first]);
    }
}
