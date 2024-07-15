#ifndef BN_OPTIMIZER_H
#define BN_OPTIMIZER_H

#include "abstract_optimizer.h"

class BnOptimizer : public AbstractOptimizer {
public:
    BnOptimizer(ModelHandler &model_handler, double max_harmonic_value);
    BnOptimizer();

    std::vector<double>& getResults();
    void temp_test();

    void optimize() override;
    void logResults() override;

    virtual ~BnOptimizer() {};


protected:

private:
    double max_harmonic_value_;
    HarmonicDriveParameterMap harmonic_drive_values_;
    std::vector<double> current_bn_values_;
    
    void setup(ModelHandler &model_handler, double max_harmonic_value);
    double fitLinearGetRoot(const std::vector<std::pair<double, double>> &points);
};

#endif // BN_OPTIMIZER_H
