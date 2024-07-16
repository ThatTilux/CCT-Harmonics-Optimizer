#include "grid_search_optimizer.h"

// Optimizes a model by performing several grid searches and extrapolating the optimal configuration for all custom CCT harmonics
GridSearchOptimizer::GridSearchOptimizer() : AbstractOptimizer()
{
    // setup
    initModel();

    // perform some model checks
    assertAllHarmonicsPresent();
    assertOnlyLinearDrives();

    // continue setup
    initCalcultor();
    computeMagnetEllBounds();
    initCriteria();
    estimateTimePerComputation();
}

// Constructor to be used for no user interaction.
GridSearchOptimizer::GridSearchOptimizer(ModelHandler &model_handler) : AbstractOptimizer()
{
    // TODO
}

// Function to get the parameter ranges for a specific component. The component is 1-indexed. Format: {{offset_min, offset_max}, {slope_min, slope_max}}
std::pair<std::pair<double, double>, std::pair<double, double>> GridSearchOptimizer::getParamRange(int component)
{
    // assert that component is between 1 and 10
    if (component < 1 || component > 10)
    {
        throw std::runtime_error("Component must be between 1 and 10.");
    }

    return param_ranges_[component - 1];
}

// Function to initialize the criteria for the grid search
void GridSearchOptimizer::initCriteria()
{
    criteria_.push_back(std::make_shared<BnObjective>());
    criteria_.push_back(std::make_shared<FittedSlopeObjective>());
}

// Function to initialize the granularities for the grid search, based on Constants::TIME_BUDGET_GRID_SEARCH. Granularity of offset is granularity of slope * 10.
void GridSearchOptimizer::computeGranularities()
{
    granularities_.resize(10);

    // for each harmonic, initialize granularities
    for (int i = 1; i <= 10; i++)
    {
        if (i == getMainComponent())
        {
            granularities_[i - 1] = {0, 0}; // dummy values for main component
            continue;
        }

        auto [offset_range, slope_range] = getParamRange(i);
        std::pair<double, double> granularities = computeGranularities(offset_range, slope_range, TIME_BUDGET_GRID_SEARCH, time_per_calc_, GRID_MIN_STEPS);
        granularities_[i - 1] = granularities;
        Logger::log_granularity(i, granularities.first, granularities.second);
    }
}

// Function to compute granularities given a time budget. Format: {offset_granularity, slope_granularity}
// Granularities are set so that the spanned grid in the parameter space is equidistant in both dimensions.
std::pair<double, double> GridSearchOptimizer::computeGranularities(std::pair<double, double> offset_range,
                                                                    std::pair<double, double> slope_range,
                                                                    double time_budget_minutes,
                                                                    double time_per_step_seconds,
                                                                    int minimum_steps)
{
    double offset_min = offset_range.first;
    double offset_max = offset_range.second;
    double slope_min = slope_range.first;
    double slope_max = slope_range.second;

    double time_budget_seconds = time_budget_minutes * 60.0;
    int num_steps = static_cast<int>(time_budget_seconds / time_per_step_seconds);

    // fulfil the minimum step requirement
    num_steps = std::max(num_steps, minimum_steps);

    double offset_span = offset_max - offset_min;
    double slope_span = slope_max - slope_min;

    double max_span = std::max(offset_span, slope_span);

    int num_steps_per_dim = static_cast<int>(std::sqrt(num_steps));

    double offset_granularity = offset_span / num_steps_per_dim;
    double slope_granularity = slope_span / num_steps_per_dim;

    // Check that both parameters have at least 2 steps
    if (offset_range.second - offset_range.first < 2 * offset_granularity)
    {
        throw std::runtime_error("Offset range is too small for the time budget.");
    }
    if (slope_range.second - slope_range.first < 2 * slope_granularity)
    {
        throw std::runtime_error("Slope range is too small for the time budget.");
    }

    return std::make_pair(offset_granularity, slope_granularity);
}

// Function to estimate the time in s it will take to run one grid search iteration
void GridSearchOptimizer::estimateTimePerComputation()
{
    Logger::info("Doing dummy computations to estimate the time...");

    // Start time
    auto start = std::chrono::high_resolution_clock::now();

    // Do some dummy computations to estimate the time
    int num_computations = 10;
    for (int i = 1; i <= num_computations; i++)
    {
        HarmonicsDataHandler handler;
        calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), handler);
    }

    // End time
    auto end = std::chrono::high_resolution_clock::now();

    // Compute the time
    std::chrono::duration<double> elapsed = end - start;
    double time_per_computation = elapsed.count() / num_computations;

    Logger::info("Estimated time per computation: " + std::to_string(time_per_computation) + " seconds");

    time_per_calc_ = time_per_computation;
}

// Function to set the parameter ranges to be around the curret cofigurations by the provided factor. New range will be [offset - factor*offset, offset + factor*offset], same for slope.
void GridSearchOptimizer::setParamRanges(double factor)
{
    // make sure that the param_ranges_ is initialized
    if (param_ranges_.empty())
    {
        param_ranges_.resize(10);
    }

    // get the current drive values
    HarmonicDriveParameterMap harmonic_drive_values = model_handler_.getHarmonicDriveValues();

    // update the param_ranges_ with the new values
    for (int i = 1; i <= 10; i++)
    {
        if (i == getMainComponent())
            continue;

        // get the current offset and slope
        double current_offset = harmonic_drive_values["B" + std::to_string(i)].getOffset();
        double current_slope = harmonic_drive_values["B" + std::to_string(i)].getSlope();

        // if one value is 0, set that range to be around some fallback value
        if (current_offset == 0)
        {
            current_offset = GRID_DRIVE_FALLBACK;
        }
        if (current_slope == 0)
        {
            current_slope = GRID_DRIVE_FALLBACK;
        }

        // get new values by multiplying by the factor
        double new_offset_min = current_offset - std::abs(current_offset) * factor;
        double new_offset_max = current_offset + std::abs(current_offset) * factor;

        double new_slope_min = current_slope - std::abs(current_slope) * factor;
        double new_slope_max = current_slope + std::abs(current_slope) * factor;

        // update the param_ranges_
        param_ranges_[i - 1] = {{new_offset_min, new_offset_max}, {new_slope_min, new_slope_max}};

        Logger::log_parameter_ranges(i, new_offset_min, new_offset_max, new_slope_min, new_slope_max);
    }
}

void GridSearchOptimizer::logResults()
{
    Logger::info("=== Grid Search Optimizer has finished ===");
    print_vector(current_bn_values_, "bn");
}

// Function to start the optimizer
void GridSearchOptimizer::optimize()
{
    Logger::info("==== Starting grid search optimizer ====");
    Logger::info("Using the following criteria:");
    for (int i = 0; i < criteria_.size(); i++)
    {
        Logger::info("Criterion " + std::to_string(i) + ": " + criteria_[i]->getLabel());
    }

    // Run the optimizer a few times with decreasing parameters: //

    // Thresholds and search factors
    std::vector<double> thresholds = {GRID_BN_THRESHOLD, 1, 0.1, 0.01};
    std::vector<double> search_factors = {GRID_SEARCH_FACTOR, GRID_SEARCH_FACTOR, GRID_SEARCH_FACTOR / 10, GRID_SEARCH_FACTOR / 100};

    // assert that they have the same length
    if (thresholds.size() != search_factors.size())
    {
        throw std::runtime_error("Thresholds and search factors must have the same length.");
    }

    // Run the optimizer with the thresholds and search factors
    for (int i = 0; i < thresholds.size(); i++)
    {
        // export the model of the previous run
        if (i != 0)
            exportModel();

        // Adjust parameter ranges to be around current configuration
        setParamRanges(search_factors[i]);

        // Recompute Granularities based on param ranges
        computeGranularities();

        // Run the optimization
        optimize(thresholds[i]);
    }
}

// Function to optimize all harmonics for the class-wide criteria in interations until all harmonics' bn values are below the provided threshold
void GridSearchOptimizer::optimize(double bn_threshold)
{
    Logger::info("=== Optimizing all harmonics with bn threshold " + std::to_string(bn_threshold) + " ===");

    // flag that all bn values are below a certain threshold
    bool allHarmonicsBelowThreshold;
    // flag to keep track of the first iteration
    bool firstIteration = true;

    // run grid searches until allHarmonicsBelowThreshold is true
    do
    {
        allHarmonicsBelowThreshold = true;

        // save the harmonic drive values. If they do not change in one iteration, the optimization may be stuck
        HarmonicDriveParameterMap drive_values_before_loop = model_handler_.getHarmonicDriveValues();

        // run grid searches
        for (int i = 1; i <= 10; i++)
        {
            // do not optimize main component
            if (i == getMainComponent())
                continue;

            // check if the bn value is good enough already. In the first iteration, optimize all
            double prev_bn = !firstIteration && current_bn_values_.size() >= i ? current_bn_values_[i - 1] : std::numeric_limits<double>::infinity();
            if (std::abs(prev_bn) < bn_threshold)
            {
                Logger::info("== Harmonic B" + std::to_string(i) + " is already below the threshold. Skipping. ==");
                continue;
            }
            else
            {
                // there is at least one harmonic not below the threshold
                allHarmonicsBelowThreshold = false;
            }

            // save the current drive values. If the new bn value is worse, revert to these values
            HarmonicDriveParameterMap prev_drive_values = model_handler_.getHarmonicDriveValues();

            // run the grid search
            std::vector<GridSearchResult> results;
            runGridSearch(i, results);

            // export the results to csv
            export_grid_search_results_to_csv(results, GRID_SEARCH_OUTPUT_DIR + "grid_search_results_B" + std::to_string(i) + ".csv");

            // Extrapolate the optimal configuration
            auto [new_offset, new_slope] = extrapolateOptimalConfiguration(results);
            Logger::log_extrapolated_values(i, new_offset, new_slope);

            // Update the model with the new configuration
            HarmonicDriveParameterMap new_config;
            new_config["B" + std::to_string(i)] = HarmonicDriveParameters(new_offset, new_slope);
            model_handler_.apply_params(new_config);

            // Recompute bn
            recompute_bn();

            // Check if the bn value actually got better
            bool improve = checkBnValue(i, prev_bn, prev_drive_values);

            // print the bn values.
            print_vector(current_bn_values_, "bn");

            // Recompute ell bounds
            computeMagnetEllBounds();
        }

        // assert that at least one drive value has changed. If not, the optimization may be stuck
        if (!hasDriveValueChanged(drive_values_before_loop))
        {
            Logger::info("No harmonic drive values have changed in one iteration. Exiting.");
            return;
        }

        firstIteration = false;
    } while (!allHarmonicsBelowThreshold);

    Logger::info("================================");
}

void GridSearchOptimizer::recompute_bn()
{
    HarmonicsDataHandler handler;
    calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), handler);
    current_bn_values_ = handler.get_bn();
}

bool GridSearchOptimizer::hasDriveValueChanged(HarmonicDriveParameterMap &drive_values_before_loop)
{
    // get the current drive values after the loop
    HarmonicDriveParameterMap drive_values_after_loop = model_handler_.getHarmonicDriveValues();

    // check if at least one value has changed
    if (drive_values_before_loop == drive_values_after_loop)
    {
        return false;
    }
    return true;
}

// Function to check if the new configuration improved the bn value. If not, revert to the previous configuration
bool GridSearchOptimizer::checkBnValue(int component, double prev_bn, HarmonicDriveParameterMap &prev_drive_values)
{
    double new_bn = current_bn_values_[component - 1];
    if (std::abs(new_bn) < std::abs(prev_bn))
    {
        Logger::info("New bn value for harmonic B" + std::to_string(component) + ": " + std::to_string(new_bn) + ". The value improved.");
        return true;
    }
    else if (std::abs(new_bn) == std::abs(prev_bn))
    {
        Logger::info("New bn value for harmonic B" + std::to_string(component) + ": " + std::to_string(new_bn) + ". The value stayed the same.");
        return false;
    }
    else
    {
        Logger::warn("New bn value for harmonic B" + std::to_string(component) + ": " + std::to_string(new_bn) + ". The value did not improve. Reverting...");
        // Revert the harmonic
        model_handler_.apply_params(prev_drive_values);
        recompute_bn();
        Logger::log_reverted_config(component, prev_drive_values["B" + std::to_string(component)].getOffset(), prev_drive_values["B" + std::to_string(component)].getSlope());
        return false;
    }
}

void GridSearchOptimizer::runGridSearch(int component, std::vector<GridSearchResult> &results)
{
    // get the parameter range for the current harmonic
    auto [offset_range, slope_range] = getParamRange(component);

    // run grid search
    std::pair<double, double> granularities = granularities_[component - 1];
    double mag_ell_start = getMinMagnetEll();
    double mag_ell_end = getMaxMagnetEll();
    GridSearch grid_search(model_handler_, calculator_, component, offset_range, slope_range, granularities.first, granularities.second, results, criteria_, mag_ell_start, mag_ell_end, time_per_calc_);
}

// Function to extrapolate the optimal configuration from the grid search results. The optimal offset, slope configuration is the one that minimizes all objectives (criteria).
std::pair<double, double> GridSearchOptimizer::extrapolateOptimalConfiguration(std::vector<GridSearchResult> &results)
{
    // make sure there are at least 2 criteria
    if (criteria_.size() < 2)
    {
        throw std::runtime_error("At least 2 criteria are needed for extrapolation.");
    }

    // linear functions for all criteria
    std::vector<std::pair<double, double>> linear_functions;

    for (int i = 0; i < criteria_.size(); i++)
    {
        // Model each data as a 2-d plane in the [offset, slope, criteria] space. Plane: z=ax+by+c
        auto [a, b, c] = StatisticalAnalysis::fitPlaneToData(results, i);
        Logger::debug("Plane coefficients for criterion " + std::to_string(i) + ": a=" + std::to_string(a) + ", b=" + std::to_string(b) + ", c=" + std::to_string(c));

        // From each plane, extract the linear function where the plane has the z value 0
        auto [offset, slope] = StatisticalAnalysis::planeToLinearFunction(a, b, c);
        Logger::debug("Linear function for criterion " + std::to_string(i) + ": Offset=" + std::to_string(offset) + ", Slope=" + std::to_string(slope));

        linear_functions.push_back({offset, slope});
    }

    if (criteria_.size() > 3)
    {
        throw std::runtime_error("Extrapolation for more than 3 criteria is not implemented.");
    }

    // Calculate the intersection of the linear functions
    auto intersection = StatisticalAnalysis::findIntersection(linear_functions[0], linear_functions[1]);

    // Check if they intersect
    if (!intersection)
    {
        throw std::runtime_error("No intersection found for the linear functions.");
    }

    Logger::debug("Intersection of the linear functions: Offset=" + std::to_string(intersection->first) + ", Slope=" + std::to_string(intersection->second));

    // Return the new values
    return *intersection;
}