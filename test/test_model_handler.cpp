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
        ModelHandler handler(test_file);
        temp_json_path = handler.getTempJsonPath();
    });

    // Check if the temp directory and JSON file were created
    EXPECT_TRUE(boost::filesystem::exists(temp_dir));
    EXPECT_TRUE(boost::filesystem::exists(temp_json_path));
}

bool containsParameterValue(HarmonicDriveParameterMap map, std::string name, HarmonicDriveParameterType type, double value, double margin)
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
    ModelHandler handler(test_file);
    HarmonicDriveParameterMap harmonic_drive_values;

    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    // Verify the parsed values
    EXPECT_EQ(harmonic_drive_values.size(), 9);

    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B1", HarmonicDriveParameterType::Constant, 3.0274872794616347e-05, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B3", HarmonicDriveParameterType::Constant, -0.00018617604979581347, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B4", HarmonicDriveParameterType::Constant, -0.00024645416164351607, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B5", HarmonicDriveParameterType::Constant, -0.00020169498553400584, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B6", HarmonicDriveParameterType::Constant, -0.001462563623493985, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B7", HarmonicDriveParameterType::Constant, 0, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B8", HarmonicDriveParameterType::Constant, 0, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B9", HarmonicDriveParameterType::Constant, 0, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B10", HarmonicDriveParameterType::Constant, 0, 1e-6));
}

// Test the setHarmonicDriveValue method (aplitude = constant)
TEST_F(ModelHandlerTest, SetHarmonicDriveValueConstant)
{
    ModelHandler handler(test_file);
    double new_value = 1.23456789;

    ASSERT_NO_THROW({
        handler.setHarmonicDriveValue("B1", HarmonicDriveParameters(new_value, HarmonicDriveParameterType::Constant));
    });

    // Verify the updated value
    HarmonicDriveParameterMap harmonic_drive_values;
    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B1", HarmonicDriveParameterType::Constant, 1.23456789, 1e-6));
}

// Test the setHarmonicDriveValue method (aplitude = linear)
TEST_F(ModelHandlerTest, SetHarmonicDriveValueLinear)
{
    ModelHandler handler(test_file_2);
    double new_slope = 1.23456789;
    double new_offset = 2.23456789;

    ASSERT_NO_THROW({
        handler.setHarmonicDriveValue("B5", HarmonicDriveParameters(new_slope, HarmonicDriveParameterType::Slope));
        handler.setHarmonicDriveValue("B5", HarmonicDriveParameters(new_offset, HarmonicDriveParameterType::Offset));
    });

    // Verify the updated value
    HarmonicDriveParameterMap harmonic_drive_values;
    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B5", HarmonicDriveParameterType::Slope, 1.23456789, 1e-6));
    EXPECT_TRUE(containsParameterValue(harmonic_drive_values, "B5", HarmonicDriveParameterType::Offset, 2.23456789, 1e-6));
}

// Test to ensure that no files in test_data directory are modified after tests
TEST_F(ModelHandlerTest, NoModificationOfOriginalFiles)
{
    // Get file size and modification time of the original JSON file
    auto original_size = boost::filesystem::file_size(test_file);
    auto original_time = boost::filesystem::last_write_time(test_file);

    // Perform operations that modify the temporary file
    ModelHandler handler(test_file);
    double new_value = 1.23456789;
    handler.setHarmonicDriveValue("quad", HarmonicDriveParameters(new_value, HarmonicDriveParameterType::Constant));

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
    ModelHandler handler(test_file_2);

    // create a map with new params
    HarmonicDriveParameterMap params;
    params["B1"] = HarmonicDriveParameters(1.23456789, HarmonicDriveParameterType::Constant);
    params["B3"] = HarmonicDriveParameters(3.23456789, HarmonicDriveParameterType::Constant);
    params["B4"] = HarmonicDriveParameters(4.23456789, HarmonicDriveParameterType::Constant);

    params["B5"] = HarmonicDriveParameters(5.23456789, 6.23456789);

    params["B6"] = HarmonicDriveParameters(7.23456789, HarmonicDriveParameterType::Constant);
    params["B7"] = HarmonicDriveParameters(8.23456789, HarmonicDriveParameterType::Constant);
    params["B8"] = HarmonicDriveParameters(9.23456789, HarmonicDriveParameterType::Constant);
    params["B9"] = HarmonicDriveParameters(10.23456789, HarmonicDriveParameterType::Constant);
    params["B10"] = HarmonicDriveParameters(11.23456789, HarmonicDriveParameterType::Constant);

    handler.apply_params(params);

    HarmonicDriveParameterMap new_drive_values = handler.getHarmonicDriveValues();

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
