#include "gtest/gtest.h"
#include "model_handler.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include <constants.h>

// Test fixture for ModelHandler tests
class ModelHandlerTest : public ::testing::Test {
protected:
    boost::filesystem::path test_file;
    boost::filesystem::path temp_dir;
    boost::filesystem::path temp_json_path;

    // Setup before each test
    void SetUp() override {
        test_file = TEST_DATA_DIR + "quad_test.json";
        temp_dir = boost::filesystem::temp_directory_path() / "model_temp";

        // Ensure the temp directory is clean
        if (boost::filesystem::exists(temp_dir)) {
            boost::filesystem::remove_all(temp_dir);
        }
    }

    // Cleanup after each test
    void TearDown() override {
        if (boost::filesystem::exists(temp_dir)) {
            boost::filesystem::remove_all(temp_dir);
        }
    }
};

// Test the constructor and createTemporaryFolder method
TEST_F(ModelHandlerTest, ConstructorAndCreateTemporaryFolder) {
    ASSERT_NO_THROW({
        ModelHandler handler(test_file);
        temp_json_path = handler.getTempJsonPath();
    });

    // Check if the temp directory and JSON file were created
    EXPECT_TRUE(boost::filesystem::exists(temp_dir));
    EXPECT_TRUE(boost::filesystem::exists(temp_json_path));
}

bool containsPair(const std::vector<std::pair<int, double>>& pairs, int first, double second, double margin) {
        for (const auto& pair : pairs) {
            if (pair.first == first && std::abs(pair.second - second) <= margin) {
                return true;
            }
        }
        return false;
    }

// Test the getHarmonicDriveValues method
TEST_F(ModelHandlerTest, GetHarmonicDriveValues) {
    ModelHandler handler(test_file);
    std::vector<std::pair<int, double>> harmonic_drive_values;
    
    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    // Verify the parsed values
    EXPECT_EQ(harmonic_drive_values.size(), 9);

    EXPECT_TRUE(containsPair(harmonic_drive_values, 1, 3.0274872794616347e-05, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 3, -0.00018617604979581347, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 4, -0.00024645416164351607, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 5, -0.00020169498553400584, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 6, -0.001462563623493985, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 7, 0, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 8, 0, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 9, 0, 1e-6));
    EXPECT_TRUE(containsPair(harmonic_drive_values, 10, 0, 1e-6));
}

// Test the setHarmonicDriveValue method
TEST_F(ModelHandlerTest, SetHarmonicDriveValue) {
    ModelHandler handler(test_file);
    double new_value = 1.23456789;

    ASSERT_NO_THROW({
        handler.setHarmonicDriveValue("B1", new_value);
    });

    // Verify the updated value
    std::vector<std::pair<int, double>> harmonic_drive_values;
    ASSERT_NO_THROW({
        harmonic_drive_values = handler.getHarmonicDriveValues("B");
    });

    EXPECT_TRUE(containsPair(harmonic_drive_values, 1, 1.23456789, 1e-6));
}

// Test to ensure that no files in test_data directory are modified after tests
TEST_F(ModelHandlerTest, NoModificationOfOriginalFiles) {
    // Get file size and modification time of the original JSON file
    auto original_size = boost::filesystem::file_size(test_file);
    auto original_time = boost::filesystem::last_write_time(test_file);

    // Perform operations that modify the temporary file
    ModelHandler handler(test_file);
    double new_value = 1.23456789;
    handler.setHarmonicDriveValue("quad", new_value);

    // Check file size and modification time of the original JSON file again
    auto final_size = boost::filesystem::file_size(test_file);
    auto final_time = boost::filesystem::last_write_time(test_file);

    // Ensure that the original file was not modified
    EXPECT_EQ(original_size, final_size);
    EXPECT_EQ(original_time, final_time);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
