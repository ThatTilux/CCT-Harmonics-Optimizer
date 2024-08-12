// AbstractOptimizer.h
#ifndef ABSTRACTOPTIMIZER_H
#define ABSTRACTOPTIMIZER_H

#include <iostream>
#include <boost/filesystem.hpp>
#include "input_output.h"
#include "model_calculator.h"
#include "statistical_analysis.h"

class AbstractOptimizer
{
public:
    AbstractOptimizer(bool disable_user_interaction = false);


    virtual void exportModel();
    virtual void optimize() = 0;
    virtual void logResults() = 0;
    virtual ~AbstractOptimizer(){};

protected:
    ModelHandler &initModel();
    void initCalculator();

    double getMaxHarmonicValue();
    double getMinMagnetEll();
    double getMaxMagnetEll();
    double getMagnetLength();
    int getMainComponent();
    void computeMagnetEllBounds();
    HarmonicDriveParameterMap initHarmonicDrives();
    void assertOnlyLinearDrives();
    void assertAllHarmonicsPresent();
    void checkMainComponent();

    boost::filesystem::path json_file_path_;
    ModelHandler model_handler_;
    ModelCalculator calculator_;
    bool disable_user_interaction_;
    std::string harmonic_drive_prefix_ = "B";

private:
    void getModelSelection();
    // min and max values in mm for the axis z position relative to ell.
    std::pair<double, double> cct_ell_bounds_ = {0, 0};
    int main_component_ = -1;
};

#endif // ABSTRACTOPTIMIZER_H
