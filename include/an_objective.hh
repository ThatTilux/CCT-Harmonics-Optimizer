#ifndef AN_OBJECTIVE_HH
#define AN_OBJECTIVE_HH

#include "abstract_objective.hh"

// Class to define the an value of the component as the objective
class AnObjective : public AbstractObjective
{
public:
    AnObjective()
    {
        label_ = "an";
    };

    double evaluate(CCTools::HarmonicsDataHandler harmonics_handler, int component) override
    {
        return harmonics_handler.get_an()[component - 1];
    }

    virtual ~AnObjective(){};
};

#endif // AN_OBJECTIVE_HH