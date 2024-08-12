#ifndef BN_OPTIMIZER_H
#define BN_OPTIMIZER_H

#include "linear_optimizer.h"

class BnOptimizer : public LinearOptimizer {
public:
    BnOptimizer(ModelHandler &model_handler, double max_value);
    BnOptimizer();

    virtual ~BnOptimizer() {};

protected:
    std::vector<double> getValues(HarmonicsDataHandler &harmonics_handler) override;
};

#endif // BN_OPTIMIZER_H
