// AbstractOptimizer.h
#ifndef ABSTRACTOPTIMIZER_H
#define ABSTRACTOPTIMIZER_H

#include <iostream>
#include <boost/filesystem.hpp>
#include "input_output.h"
#include "harmonics_calculator.h"

class AbstractOptimizer {
public:
    AbstractOptimizer(bool disable_user_interaction = false);

    static std::pair<double, double> linearRegression(const std::vector<std::pair<double, double>> &points);

    virtual void optimize() = 0;
    virtual void logResults() = 0;
    virtual ~AbstractOptimizer() {};

protected:
    ModelHandler& initModel();
    void initCalcultor();

    double getMaxHarmonicValue();
    HarmonicDriveParameterMap initHarmonicDrives();

    boost::filesystem::path json_file_path_; 
    ModelHandler model_handler_;
    HarmonicsCalculator calculator_;
    bool disable_user_interaction_;

private:
    void getModelSelection();
};

#endif // ABSTRACTOPTIMIZER_H
