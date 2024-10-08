#include "gtest/gtest.h"
#include "harmonic_drive_parameter.h"
#include <stdexcept>

class HarmonicDriveParametersTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup required for this test
    }

    void TearDown() override {
        // No teardown required for this test
    }
};

TEST_F(HarmonicDriveParametersTest, EqualityOperator) {
    CCTools::HarmonicDriveParameters params1(0.1, 0.2);
    CCTools::HarmonicDriveParameters params2(0.1, 0.2);
    CCTools::HarmonicDriveParameters params3(0.1, 0.3);

    ASSERT_TRUE(params1 == params2);
    ASSERT_FALSE(params1 == params3);
}

TEST_F(HarmonicDriveParametersTest, SetValueFunction) {
    // Test for Offset type
    CCTools::HarmonicDriveParameters params_offset(0.1, CCTools::HarmonicDriveParameterType::Offset);
    params_offset.setValue(0.2, CCTools::HarmonicDriveParameterType::Offset);
    ASSERT_DOUBLE_EQ(params_offset.getOffset(), 0.2);
    ASSERT_TRUE(params_offset.isOffset());
    ASSERT_FALSE(params_offset.isSlope());
    ASSERT_FALSE(params_offset.isConstant());

    // Test for Slope type
    CCTools::HarmonicDriveParameters params_slope(0.1, CCTools::HarmonicDriveParameterType::Slope);
    params_slope.setValue(0.2, CCTools::HarmonicDriveParameterType::Slope);
    ASSERT_DOUBLE_EQ(params_slope.getSlope(), 0.2);
    ASSERT_FALSE(params_slope.isOffset());
    ASSERT_TRUE(params_slope.isSlope());
    ASSERT_FALSE(params_slope.isConstant());

    // Test for Constant type
    CCTools::HarmonicDriveParameters params_constant(0.1, CCTools::HarmonicDriveParameterType::Constant);
    params_constant.setValue(0.2, CCTools::HarmonicDriveParameterType::Constant);
    ASSERT_DOUBLE_EQ(params_constant.getConstant(), 0.2);
    ASSERT_FALSE(params_constant.isOffset());
    ASSERT_FALSE(params_constant.isSlope());
    ASSERT_TRUE(params_constant.isConstant());

    // Test for OffsetAndSlope type
    CCTools::HarmonicDriveParameters params_offset_slope(0.1, 0.2);
    params_offset_slope.setValue(0.3, CCTools::HarmonicDriveParameterType::Offset);
    params_offset_slope.setValue(0.4, CCTools::HarmonicDriveParameterType::Slope);
    ASSERT_DOUBLE_EQ(params_offset_slope.getOffset(), 0.3);
    ASSERT_DOUBLE_EQ(params_offset_slope.getSlope(), 0.4);
    ASSERT_TRUE(params_offset_slope.isOffset());
    ASSERT_TRUE(params_offset_slope.isSlope());
    ASSERT_FALSE(params_offset_slope.isConstant());
}


TEST_F(HarmonicDriveParametersTest, IsMethods) {
    CCTools::HarmonicDriveParameters params1(0.1, CCTools::HarmonicDriveParameterType::Offset);
    ASSERT_TRUE(params1.isOffset());
    ASSERT_FALSE(params1.isSlope());
    ASSERT_FALSE(params1.isConstant());

    CCTools::HarmonicDriveParameters params2(0.2, CCTools::HarmonicDriveParameterType::Slope);
    ASSERT_FALSE(params2.isOffset());
    ASSERT_TRUE(params2.isSlope());
    ASSERT_FALSE(params2.isConstant());

    CCTools::HarmonicDriveParameters params3(0.3, CCTools::HarmonicDriveParameterType::Constant);
    ASSERT_FALSE(params3.isOffset());
    ASSERT_FALSE(params3.isSlope());
    ASSERT_TRUE(params3.isConstant());

    CCTools::HarmonicDriveParameters params4(0.1, 0.2);
    ASSERT_TRUE(params4.isOffset());
    ASSERT_TRUE(params4.isSlope());
    ASSERT_FALSE(params4.isConstant());
}

TEST_F(HarmonicDriveParametersTest, GetMethods) {
    CCTools::HarmonicDriveParameters params1(0.1, CCTools::HarmonicDriveParameterType::Offset);
    ASSERT_DOUBLE_EQ(params1.getOffset(), 0.1);
    ASSERT_THROW(params1.getSlope(), std::logic_error);
    ASSERT_THROW(params1.getConstant(), std::logic_error);

    CCTools::HarmonicDriveParameters params2(0.2, CCTools::HarmonicDriveParameterType::Slope);
    ASSERT_DOUBLE_EQ(params2.getSlope(), 0.2);
    ASSERT_THROW(params2.getOffset(), std::logic_error);
    ASSERT_THROW(params2.getConstant(), std::logic_error);

    CCTools::HarmonicDriveParameters params3(0.3, CCTools::HarmonicDriveParameterType::Constant);
    ASSERT_DOUBLE_EQ(params3.getConstant(), 0.3);
    ASSERT_THROW(params3.getOffset(), std::logic_error);
    ASSERT_THROW(params3.getSlope(), std::logic_error);
}

TEST_F(HarmonicDriveParametersTest, GetTypeMethod) {
    CCTools::HarmonicDriveParameters params1(0.1, CCTools::HarmonicDriveParameterType::Offset);
    ASSERT_DOUBLE_EQ(params1.get(CCTools::HarmonicDriveParameterType::Offset), 0.1);
    ASSERT_THROW(params1.get(CCTools::HarmonicDriveParameterType::Slope), std::runtime_error);
    ASSERT_THROW(params1.get(CCTools::HarmonicDriveParameterType::Constant), std::runtime_error);

    CCTools::HarmonicDriveParameters params2(0.2, CCTools::HarmonicDriveParameterType::Slope);
    ASSERT_DOUBLE_EQ(params2.get(CCTools::HarmonicDriveParameterType::Slope), 0.2);
    ASSERT_THROW(params2.get(CCTools::HarmonicDriveParameterType::Offset), std::runtime_error);
    ASSERT_THROW(params2.get(CCTools::HarmonicDriveParameterType::Constant), std::runtime_error);

    CCTools::HarmonicDriveParameters params3(0.3, CCTools::HarmonicDriveParameterType::Constant);
    ASSERT_DOUBLE_EQ(params3.get(CCTools::HarmonicDriveParameterType::Constant), 0.3);
    ASSERT_THROW(params3.get(CCTools::HarmonicDriveParameterType::Offset), std::runtime_error);
    ASSERT_THROW(params3.get(CCTools::HarmonicDriveParameterType::Slope), std::runtime_error);

    CCTools::HarmonicDriveParameters params4(0.1, 0.2);
    ASSERT_DOUBLE_EQ(params4.get(CCTools::HarmonicDriveParameterType::Offset), 0.1);
    ASSERT_DOUBLE_EQ(params4.get(CCTools::HarmonicDriveParameterType::Slope), 0.2);
    ASSERT_THROW(params4.get(CCTools::HarmonicDriveParameterType::Constant), std::runtime_error);
}
