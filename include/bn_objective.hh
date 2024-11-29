#ifndef BN_OBJECTIVE_HH
#define BN_OBJECTIVE_HH

#include "abstract_objective.hh"

/**
 * @brief Defines the absolute bn value of a component as the objective of an optimizer.
 * 
 * This class serves as an objective for optimizers that support modular objective injection.
 * This class defines the absolute bn value of a harmonic component as the objective to be minimized.
 */
class BnObjective : public AbstractObjective
{
public:

    /**
     * @brief Construct an BnObjective object.
     */
    BnObjective()
    {
        label_ = "bn";
    };
    
    /**
     * @brief Evaluate the objective.
     * @param harmonics_handler Result of the harmonics calculation
     * @param component Harmonics component to be evaluated. 1-indexed
     * 
     * This function evaluates the objective for a specific harmonics component based on the results of a harmonics calculation.
     */
    double evaluate(CCTools::HarmonicsDataHandler harmonics_handler, int component) override
    {
        return harmonics_handler.get_bn()[component - 1];
    }

    virtual ~BnObjective(){};
};

#endif // BN_OBJECTIVE_HH