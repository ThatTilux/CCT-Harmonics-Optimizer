#ifndef BN_OBJECTIVE_HH
#define BN_OBJECTIVE_HH

#include "abstract_objective.hh"

// Class to define the bn value of the component as the objective
class BnObjective : public AbstractObjective
{
public:
    BnObjective()
    {
        label_ = "bn";
    };

    double evaluate(HarmonicsDataHandler harmonics_handler, int component) override
    {
        return harmonics_handler.get_bn()[component - 1];
    }

    virtual ~BnObjective(){};
};

#endif // BN_OBJECTIVE_HH