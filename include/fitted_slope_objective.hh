#ifndef FITTED_SLOPE_OBJECTIVE_HH
#define FITTED_SLOPE_OBJECTIVE_HH

#include "abstract_objective.hh"

// Class to define the following objective: take the Bn data and fit a linear function. The objective is the slope of the fitted line
class FittedSlopeObjective : public AbstractObjective
{
public:
    FittedSlopeObjective()
    {
        label_ = "fitted_slope";
    };

    double evaluate(HarmonicsHandler harmonics_handler, int component) override
    {
        // get the Bn and ell
        std::vector<std::pair<double, double>> points = harmonics_handler.get_Bn(component);

        // apply transformations to the data
        apply_transformations(points);

        // fit a linear function
        auto [slope, intercept] = StatisticalAnalysis::linearRegression(points);

        return slope;
    }

    // Funtion to apply transformations to the ell & Bn data before fitting a linear function
    void apply_transformations(std::vector<std::pair<double, double>> &Bn_data){
        // remove all the pairs where the ell value is not inside the set bounds
        Bn_data.erase(std::remove_if(Bn_data.begin(), Bn_data.end(), [](const std::pair<double, double> &pair)
                                { return pair.first < MAG_START_POS || pair.first > MAG_END_POS; }),
                Bn_data.end());

        //make sure there are at least 2 points
        if (Bn_data.size() < 2)
        {
            throw std::runtime_error("Not enough points for linear regression.");
        }

        // scale the ell values so that the first one is -0.5 and last one 0.5
        double ell_min = Bn_data.front().first;
        double ell_max = Bn_data.back().first;
        double ell_range = ell_max - ell_min;
        for (auto &pair : Bn_data)
        {
            pair.first = (pair.first - ell_min) / ell_range - 0.5;
        }
    }

    virtual ~FittedSlopeObjective(){};
};

#endif // FITTED_SLOPE_OBJECTIVE_HH