#ifndef FITTED_SLOPE_OBJECTIVE_HH
#define FITTED_SLOPE_OBJECTIVE_HH

#include "abstract_objective.hh"
#include "constants.h"
#include "input_output.h"

/**
 * @brief Defines the slope of a component's Bn curve as an optimization objective.
 * 
 * This class serves as an objective for optimizers that support modular objective injection.
 * It defines the objective for a harmonic component as the slope of a fitted linear function to the component's Bn data.
 * The goal is to minimize this slope, favoring a constant Bn function, which indicates minimal variation in the magnetic field component along the magnet's length (ell).
 * The class also allows specifying magnet ell bounds, restricting the slope calculation to the ell range where the magnet physically is.
 */
class FittedSlopeObjective : public AbstractObjective
{
public:

    /**
     * @brief Construct an FittedSlopeObjective object.
     */
    FittedSlopeObjective()
    {
        label_ = "fitted_slope";
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
        return evaluate(harmonics_handler, component, std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
    }

    /**
     * @brief Evaluate the objective.
     * @param harmonics_handler Result of the harmonics calculation
     * @param component Harmonics component to be evaluated. 1-indexed
     * @param mag_start_pos Starting position of the magnet relative to the harmonic calculation's axis (ell).
     * @param mag_end_pos Ending position of the magnet relative to the harmonic calculation's axis (ell).
     * @param export_Bn Flag to export the Bn data to a CSV file for debugging purposes.
     * 
     * This function evaluates the objective for a specific harmonics component based on the results of a harmonics calculation.
     */
    double evaluate(CCTools::HarmonicsDataHandler harmonics_handler, int component, double mag_start_pos, double mag_end_pos, bool export_Bn = false)
    {
        // get the Bn and ell
        std::vector<std::pair<double, double>> points = harmonics_handler.get_Bn(component);

        // apply transformations to the data
        apply_transformations(points, mag_start_pos, mag_end_pos);

        if (export_Bn)
        {
            export_data_to_csv(points, GRID_SEARCH_OUTPUT_DIR + "Bn_B" + std::to_string(component) + ".csv");
        }

        // fit a linear function
        auto [slope, intercept] = StatisticalAnalysis::linearRegression(points);

        return slope;
    }

    /**
     * @brief Apply transformations to the ell & Bn data before fitting a linear function.
     * @param Bn_data Vector of pairs of ell and Bn values.
     * @param mag_start_pos Starting position of the magnet relative to the harmonic calculation's axis (ell).
     * @param mag_end_pos Ending position of the magnet relative to the harmonic calculation's axis (ell).
     * 
     * This function applies transformations to the ell & Bn data before fitting a linear function.
     * It removes all the pairs where the ell value is not inside the set bounds.
     * It scales the ell values so that the first one is -0.5 and last one 0.5.
     */
    void apply_transformations(std::vector<std::pair<double, double>> &Bn_data, double mag_start_pos, double mag_end_pos)
    {
        // remove all the pairs where the ell value is not inside the set bounds
        Bn_data.erase(std::remove_if(Bn_data.begin(), Bn_data.end(), [this, mag_start_pos, mag_end_pos](const std::pair<double, double> &pair)
                                     { return pair.first < mag_start_pos || pair.first > mag_end_pos; }),
                      Bn_data.end());

        // make sure there are at least 2 points
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

    virtual ~FittedSlopeObjective() {};
};

#endif // FITTED_SLOPE_OBJECTIVE_HH