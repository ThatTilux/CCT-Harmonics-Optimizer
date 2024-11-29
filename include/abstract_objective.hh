#ifndef ABSTRACT_OBJECTIVE_HH
#define ABSTRACT_OBJECTIVE_HH

#include <string>
#include "harmonics_data_handler.h"
#include "statistical_analysis.h"

/**
 * @brief Abstract class to define objectives for some optimizers.
 *
 * This abstract class can be implemented to define objectives for optimizers that support modular objective injection.
 * Derived objectives must compute the objective value from a HarmonicsDataHandler object.
 */
class AbstractObjective
{
public:
    /**
     * @brief Construct an AbstractObjective object.
     *
     * Default constructor for the AbstractObjective class.
     */
    AbstractObjective() {};

    /**
     * @brief Evaluate the objective.
     * @param harmonics_handler Result of the harmonics calculation
     * @param component Harmonics component to be evaluated. 1-indexed
     *
     * This function evaluates the objective for a specific harmonics component based on the results of a harmonics calculation.
     * This function is pure virtual and needs to be implemented in derived classes.
     */
    virtual double evaluate(CCTools::HarmonicsDataHandler harmonics_handler, int component) = 0;
    virtual ~AbstractObjective() {};

    /**
     * @brief Get the objective label.
     * 
     * This function returns the unique human-redable label of the objective.
     */
    std::string getLabel() const
    {
        return label_;
    }

protected:
    /**
     * @brief Label of the objective.
     * @details This unique and human-readable identifier of the objective is to be set in derived classes.
     */
    std::string label_;
};

#endif // ABSTRACT_OBJECTIVE_HH