#ifndef AN_OPTIMIZER_H
#define AN_OPTIMIZER_H

#include "linear_optimizer.h"
#include "an_objective.hh"

class AnOptimizer : public LinearOptimizer {
public:
    AnOptimizer(ModelHandler &model_handler, double max_value);
    AnOptimizer();

    virtual ~AnOptimizer() {};

protected:
    std::vector<double> getValues(HarmonicsDataHandler &harmonics_handler) override;


private:
    void setup();
    void checkForHarmonicDriveConstraints(HarmonicDriveParameterMap harmonic_drive_values);
};

#endif // AN_OPTIMIZER_H
