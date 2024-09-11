#ifndef LINEAR_OPTIMIZER_H
#define LINEAR_OPTIMIZER_H

#include "abstract_optimizer.h"
#include "abstract_objective.hh"

class LinearOptimizer : public AbstractOptimizer {
public:
    LinearOptimizer(std::string optimized_value_label, std::string harmonic_drive_prefix, ModelHandler &model_handler, double max_value);
    LinearOptimizer(std::string optimized_value_label, std::string harmonic_drive_prefix);

    std::vector<double>& getResults();

    void optimize() override;
    void logResults() override;

    virtual ~LinearOptimizer() {};

protected:
    void getDriveValueAndType(const std::string &name, double &current_drive_value, HarmonicDriveParameterType &drive_type);
    virtual std::vector<double> getValues(HarmonicsDataHandler &harmonics_handler) = 0;

    static double fitLinearGetRoot(const std::vector<std::pair<double, double>> &points);

    // label of the type of values optimized
    std::string optimized_value_label_;
    HarmonicDriveParameterMap drive_values_;

private:
    double max_value_;
    std::vector<double> current_values_;
    
    void setup(ModelHandler &model_handler, double max_value, std::string harmonic_drive_prefix);
};

#endif // LINEAR_OPTIMIZER_H
