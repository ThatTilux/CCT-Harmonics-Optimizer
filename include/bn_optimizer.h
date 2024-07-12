#ifndef BN_OPTIMIZER_H
#define BN_OPTIMIZER_H

#include "abstract_optimizer.h"

class BnOptimizer : public AbstractOptimizer {
public:
    BnOptimizer(ModelHandler &model_handler, double max_harmonic_value, bool disable_logging = false);
    BnOptimizer(bool disable_logging = false);

    std::vector<double>& getResults();

    void optimize() override;
    void logResults() override;

    virtual ~BnOptimizer() {};


protected:

private:
    double max_harmonic_value_;
    HarmonicDriveParameterMap harmonic_drive_values_;
    std::vector<double> current_bn_values_;
    bool disable_logging_ = false;
    
    void setup(ModelHandler &model_handler, double max_harmonic_value, bool disable_logging);
    double fitLinearGetRoot(const std::vector<std::pair<double, double>> &points);
};

#endif // BN_OPTIMIZER_H
