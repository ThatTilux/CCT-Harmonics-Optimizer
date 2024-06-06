#include "gtest/gtest.h"
#include "objective_function.h"
#include "constants.h"

class ObjectiveFunctionTest : public ::testing::Test
{
protected:
    boost::filesystem::path test_file;
    boost::filesystem::path test_file_2;
    boost::filesystem::path test_file_3;
    boost::filesystem::path temp_dir;

    // Setup before each test
    void SetUp() override
    {
        test_file = TEST_DATA_DIR + "quad_test.json";
        test_file_2 = TEST_DATA_DIR + "quad_test_B5_linear.json";
        test_file_3 = TEST_DATA_DIR + "quad_test_all_linear.json";
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

// Test for the constructor; it should only accept models with only harmonics with linear scaling functions
TEST_F(ObjectiveFunctionTest, ConstructorOnlyLinearScalingFunctions)
{
    ModelHandler handler(test_file_2);
    ModelHandler handler_2(test_file_3);

    // this should throw an exception
    ASSERT_THROW({ ObjectiveFunction obj(handler); }, std::runtime_error);

    // this model has only linears
    ASSERT_NO_THROW({
        ObjectiveFunction obj(handler_2);
    });
}
