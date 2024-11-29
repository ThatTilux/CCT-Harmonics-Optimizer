#ifndef AN_OBJECTIVE_HH
#define AN_OBJECTIVE_HH

#include "abstract_objective.hh"

/**
 * @brief Defines the absolute an value of a component as the objective of an optimizer.
 * 
 * This class serves as an objective for optimizers that support modular objective injection.
 * This class defines the absolute an value of a harmonic component as the objective to be minimized.
 */
class AnObjective : public AbstractObjective
{
public:

    /**
     * @brief Construct an AnObjective object.
     */
    AnObjective()
    {
        label_ = "an";
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
        return harmonics_handler.get_an()[component - 1];
    }

    virtual ~AnObjective(){};
};

#endif // AN_OBJECTIVE_HH