#ifndef ABSTRACT_OBJECTIVE_HH
#define ABSTRACT_OBJECTIVE_HH

#include <string>
#include "harmonics_data_handler.h"
#include "statistical_analysis.h"

// Abstract class to define an objective to be used in optimizers and objective functions
class AbstractObjective
{
public:
    AbstractObjective(){};

    virtual double evaluate(HarmonicsDataHandler harmonics_handler, int component) = 0;
    virtual ~AbstractObjective(){};

    std::string getLabel() const
    {
        return label_;
    }

    std::string label_;
};

#endif // ABSTRACT_OBJECTIVE_HH