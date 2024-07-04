#ifndef OBJECTIVE_FUNCTION_H
#define OBJECTIVE_FUNCTION_H

#include <vector>
#include "harmonics_calculator.h"
#include "abstract_optimizer.h"
#include "harmonics_handler.h"
#include "model_handler.h"
#include "harmonic_drive_parameter.h"
#include "constants.h"
#include <iostream>
#include <cmath>


class ObjectiveFunction{    
    public:
        ObjectiveFunction(const ModelHandler &model_handler, double weight_chisquared);

        double objective_function(HarmonicDriveParameterMap &params);
        int chiSquaredOptimizer(int component, double scaling_factor, bool temp_do_bn_optimizer);
        double objective_function_slope(HarmonicDriveParameterMap &params);

        
    private:

        boost::filesystem::path json_file_path_;
        HarmonicsCalculator calculator_;
        ModelHandler model_handler_;
        double weight_chisquared_;
};


double chiSquared(HarmonicsHandler &harmonics_handler, int component, std::pair<double, double>* fitted = nullptr);
double computeVariance(const std::vector<double> &y);
std::pair<double, double> linearRegression(const std::vector<std::pair<double, double>> &points);
void transform_drive_values(double &offset, double &slope);


#endif // OBJECTIVE_FUNCTION_H