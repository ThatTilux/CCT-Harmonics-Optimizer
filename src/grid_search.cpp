#include "grid_search.h"

GridSearch::GridSearch(ModelHandler &model_handler,
                       ModelCalculator calculator,
                       int component,
                       std::pair<double, double> &offset_range,
                       std::pair<double, double> &slope_range,
                       double &offset_granularity,
                       double &slope_granularity,
                       std::vector<GridSearchResult> &results,
                       std::vector<std::shared_ptr<AbstractObjective>> &criteria,
                       double mag_ell_start,
                       double mag_ell_end,
                       double estimated_time_per_calc)
    : model_handler_(model_handler),
      calculator_(calculator),
      component_(component),
      offset_range_(offset_range),
      slope_range_(slope_range),
      offset_granularity_(offset_granularity),
      slope_granularity_(slope_granularity),
      results_(results),
      criteria_(criteria),
      mag_ell_start_(mag_ell_start),
      mag_ell_end_(mag_ell_end),
      estimated_time_per_step_(estimated_time_per_calc)
{
    run();
}

void GridSearch::run()
{
    Logger::info("== Running grid search for harmonic B" + std::to_string(component_) + " ==");
    logParams();

    // Get the number of steps for the grid search
    // This has to be done inside this method to avoid floating point precision issues
    int counter = 0;
    for (double offset = offset_range_.first; offset <= offset_range_.second; offset += offset_granularity_)
    {
        for (double slope = slope_range_.first; slope <= slope_range_.second; slope += slope_granularity_)
        {
            counter++;
        }
    }

    total_steps_ = counter;

    logEstmatedTime();

    // Counter for the iteration number
    int iteration = 1;

    // Loop over the offset and slope ranges
    for (double offset = offset_range_.first; offset <= offset_range_.second; offset += offset_granularity_)
    {
        for (double slope = slope_range_.first; slope <= slope_range_.second; slope += slope_granularity_)
        {
            Logger::info("= Running iteration " + std::to_string(iteration) + " of " + std::to_string(total_steps_) + " =");
            Logger::info_double("Offset", offset);
            Logger::info_double("Slope", slope);

            // Apply the new parameters
            HarmonicDriveParameterMap params;
            params["B" + std::to_string(component_)] = HarmonicDriveParameters(offset, slope);
            model_handler_.apply_params(params);

            // Run harmonics calculation
            HarmonicsDataHandler harmonics_handler;
            calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), harmonics_handler);

            // Evaluate the criteria
            std::vector<double> criteria_values;
            for (auto &criterion : criteria_)
            {
                double value;
                // if the criteria is FittedSlopeObjective, pass more params
                if (criterion->getLabel() == "fitted_slope")
                {
                    value = std::dynamic_pointer_cast<FittedSlopeObjective>(criterion)->evaluate(harmonics_handler, component_, mag_ell_start_, mag_ell_end_);
                } else {
                    value = criterion->evaluate(harmonics_handler, component_);
                }
                criteria_values.push_back(value);
                Logger::info_double(criterion->getLabel(), value);
            }

            // Store the results
            GridSearchResult result;
            result.offset = offset;
            result.slope = slope;
            result.criteria_values = criteria_values;
            results_.push_back(result);

            iteration++;
        }
    }
}

void GridSearch::logParams()
{
    // log all params
    Logger::info("Grid search params:");
    Logger::info_double("Offset Min", offset_range_.first);
    Logger::info_double("Offset Max", offset_range_.second);
    Logger::info_double("Slope Min", slope_range_.first);
    Logger::info_double("Slope Max", slope_range_.second);
    Logger::info_double("Granularity Offset", offset_granularity_);
    Logger::info_double("Granularity Slope", slope_granularity_);
}

// Function to estimate and log the time it will take to run the grid search
void GridSearch::logEstmatedTime()
{
    Logger::info("Grid search will run " + std::to_string(total_steps_) + " iterations.");

    double total_time = total_steps_ * estimated_time_per_step_;
    double time_minutes = total_time / 60;
    double time_hours = time_minutes / 60;

    Logger::info("Estimated time to run grid search: " + std::to_string(time_minutes) + " minutes, equal to " + std::to_string(time_hours) + " hours.");
}