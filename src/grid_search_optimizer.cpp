#include "grid_search_optimizer.h"

// Optimizes a model by performing several grid searches and extrapolating the optimal configuration for all custom CCT harmonics
GridSearchOptimizer::GridSearchOptimizer(std::vector<std::shared_ptr<AbstractObjective>> criteria,
                                         std::vector<double> thresholds, std::vector<double> search_factors,
                                         const int grid_num_steps,
                                         std::vector<int> harmonics_to_optimize) : AbstractOptimizer(),
                                                                                   criteria_(criteria), thresholds_(thresholds), search_factors_(search_factors), harmonics_to_optimize_(harmonics_to_optimize),
                                                                                   grid_num_steps_(grid_num_steps)
{
    // setup
    initModel();
    setup();
}

// Constructor with no user interaction - to be used for testing
GridSearchOptimizer::GridSearchOptimizer(ModelHandler &model_handler, std::vector<std::shared_ptr<AbstractObjective>> criteria,
                                         std::vector<double> thresholds, std::vector<double> search_factors,
                                         const int grid_num_steps,
                                         std::vector<int> harmonics_to_optimize) : AbstractOptimizer(true), criteria_(criteria), thresholds_(thresholds), search_factors_(search_factors), harmonics_to_optimize_(harmonics_to_optimize),
                                                                                   grid_num_steps_(grid_num_steps)
{
    this->model_handler_ = model_handler;
    setup();
}

// Setup function called from the constructors
void GridSearchOptimizer::setup()
{
    // perform some model checks
    assertAllHarmonicsPresent();
    checkMainComponent();
    assertOnlyLinearDrives();

    // continue setup
    initCalculator();
    computeMagnetEllBounds();
}

// Function to get the parameter ranges for a specific component. The component is 1-indexed. Format: {{offset_min, offset_max}, {slope_min, slope_max}}
std::pair<std::pair<double, double>, std::pair<double, double>> GridSearchOptimizer::getParamRange(int component)
{
    // assert that component is between 1 and 10
    if (component < 1 || component > 10)
    {
        throw std::runtime_error("Component must be between 1 and 10 when getting parameter ranges.");
    }

    return param_ranges_[component - 1];
}

// Function to manually inject param ranges. This will completely overwrite any current or default ranges.
void GridSearchOptimizer::injectParamRanges(std::vector<std::pair<std::pair<double, double>, std::pair<double, double>>> param_ranges)
{
    // make sure that a param range is provided for every harmonic
    if (param_ranges.size() != 10)
    {
        throw std::runtime_error("A parameter range must be injected for every harmonic B1-B10. Use dummy values for harmonics not to be optimized.");
    }

    param_ranges_ = param_ranges;
    injected_param_ranges_ = true;

    Logger::debug("Injected parameter ranges.");
}

// Function to change the number of grid search steps after initialzation
void GridSearchOptimizer::setNumSteps(int num_steps){
    grid_num_steps_ = num_steps;
}


// Function to initialize the granularities for the grid search, based on the set time limit.
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
        std::pair<double, double> granularities = computeGranularities(offset_range, slope_range);
        granularities_[i - 1] = granularities;
        Logger::log_granularity(i, granularities.first, granularities.second);
    }
}

// Function to compute granularities given a time budget. Format: {offset_granularity, slope_granularity}
// Granularities are set so that the spanned grid in the parameter space is equidistant in both dimensions.
std::pair<double, double> GridSearchOptimizer::computeGranularities(std::pair<double, double> offset_range,
                                                                    std::pair<double, double> slope_range)
{
    double offset_min = offset_range.first;
    double offset_max = offset_range.second;
    double slope_min = slope_range.first;
    double slope_max = slope_range.second;

    int num_steps = grid_num_steps_;


    double offset_span = offset_max - offset_min;
    double slope_span = slope_max - slope_min;

    double max_span = std::max(offset_span, slope_span);

    int num_steps_per_dim = static_cast<int>(std::sqrt(num_steps));

    double offset_granularity = offset_span / num_steps_per_dim;
    double slope_granularity = slope_span / num_steps_per_dim;

    // Check that both parameters have at least 2 steps
    if (offset_range.second - offset_range.first < 2 * offset_granularity)
    {
        throw std::runtime_error("Offset does not have at least 2 steps. Something went wrong.");
    }
    if (slope_range.second - slope_range.first < 2 * slope_granularity)
    {
        throw std::runtime_error("Slope does not have at least 2 steps. Something went wrong.");
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
    int num_computations = 5;
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
    // if ranges were manually injected, do not compute new ones
    if (injected_param_ranges_)
    {
        Logger::debug("Parameter ranges were manually injected. Not computing new ones.");
        return;
    }

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

// Function to export the optimized model and log as interim result
void GridSearchOptimizer::exportModel()
{
    // Save and get path
    const boost::filesystem::path path = copyModelWithTimestamp(model_handler_.getTempJsonPath());

    // Get the bn values
    HarmonicsDataHandler handler;
    calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), handler);
    std::vector<double> bn_values = handler.get_bn();

    // Log the interim result
    InterimResult result;
    result.file_path = path.string();
    result.bn_values = bn_values;
    interim_results_.push_back(result);
}

void GridSearchOptimizer::logResults()
{
    Logger::info("==== Grid Search Optimizer has finished ====");
    Logger::info("Several iterations have been run with decreasing parameters.");
    Logger::info("After every iteration, the interim model has been saved:");

    for (int i = 0; i < interim_results_.size(); i++)
    {
        if (i == interim_results_.size() - 1){
            Logger::info("==== Final model ====");
        } else {
            Logger::info("==== Interim result " + std::to_string(i+1) + " ====");
        }
        Logger::info("File location: " + interim_results_[i].file_path);
        print_vector(interim_results_[i].bn_values, "bn");
    }
}

// Function to start the optimizer
void GridSearchOptimizer::optimize()
{
    estimateTimePerComputation();
    Logger::info("==== Starting grid search optimizer ====");
    Logger::info("Using the following criteria:");
    for (int i = 0; i < criteria_.size(); i++)
    {
        Logger::info("Criterion " + std::to_string(i) + ": " + criteria_[i]->getLabel());
    }

    // Run the optimizer a few times with decreasing parameters: //

    if (thresholds_.size() != search_factors_.size())
    {
        throw std::runtime_error("Thresholds and search factors must have the same length for the grid search optimizer.");
    }

    // Run the optimizer with the thresholds and search factors
    for (int i = 0; i < thresholds_.size(); i++)
    {
        // export the model of the previous run
        if (i != 0)
            exportModel();

        // Adjust parameter ranges to be around current configuration
        setParamRanges(search_factors_[i]);

        // Recompute Granularities based on param ranges
        computeGranularities();

        // Run the optimization
        optimize(thresholds_[i]);
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
        for (int i : harmonics_to_optimize_)
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
            auto [new_offset, new_slope] = extrapolateOptimalConfiguration(results, prev_drive_values["B" + std::to_string(i)]);
            Logger::log_extrapolated_values(i, new_offset, new_slope);

            // Update the model with the new configuration
            HarmonicDriveParameterMap new_config;
            new_config["B" + std::to_string(i)] = HarmonicDriveParameters(new_offset, new_slope);
            model_handler_.apply_params(new_config);

            // Recompute bn
            recompute_bn();

            // Check if the bn value actually got better
            checkBnValue(i, prev_bn, prev_drive_values);

            // print the bn values.
            print_vector(current_bn_values_, "bn");

            // Recompute ell bounds
            computeMagnetEllBounds();

            // check sanity of the model
            checkLengthSanity(prev_drive_values);
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

// Function to check the sanity of the magnet model. Will revert to the fallback config if the length of the magnet changed considerably.
void GridSearchOptimizer::checkLengthSanity(HarmonicDriveParameterMap &fallback_drives)
{
    static double previous_length = 0;

    // get the current length
    double current_length = getMagnetLength();

    // compare; sane if the length has not changed by more than 30%
    if (previous_length != 0 && std::abs(current_length - previous_length) > std::abs(0.3 * previous_length))
    {
        Logger::warn("The length of the magnet has changed considerably from " + std::to_string(previous_length) + " mm to " + std::to_string(current_length) + " mm. Reverting to the previous configuration.");
        model_handler_.apply_params(fallback_drives);
        recompute_bn();
        computeMagnetEllBounds();
    }
    else
    {
        previous_length = current_length;
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
std::pair<double, double> GridSearchOptimizer::extrapolateOptimalConfiguration(std::vector<GridSearchResult> &results, HarmonicDriveParameters &current_drive)
{
    // make sure there are at least 1 criterion
    if (criteria_.size() < 1)
    {
        throw std::runtime_error("At least 1 criterion is needed for extrapolation.");
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

    // Get the optimal config using the linear functions of the criteria

    if (criteria_.size() == 1)
    {
        return extrapolateOptimalConfiguration(linear_functions[0], current_drive);
    }

    if (criteria_.size() == 2)
    {
        return extrapolateOptimalConfiguration(linear_functions[0], linear_functions[1]);
    }

    throw std::runtime_error("Extrapolation for more than 2 criteria is not implemented.");
}

// Function to extrapolate the optimal config given the linear functions of 2 criteria
std::pair<double, double> GridSearchOptimizer::extrapolateOptimalConfiguration(std::pair<double, double> linear_function1, std::pair<double, double> linear_function2)
{
    // Calculate the intersection of the linear functions
    auto intersection = StatisticalAnalysis::findIntersection(linear_function1, linear_function2);

    // Check if they intersect
    if (!intersection)
    {
        throw std::runtime_error("No intersection found for the linear functions.");
    }

    Logger::debug("Intersection of the linear functions: Offset=" + std::to_string(intersection->first) + ", Slope=" + std::to_string(intersection->second));

    // Return the new values
    return *intersection;
}

// Function to extrapolate the optimal config given the linear functions of 1 criterion
std::pair<double, double> GridSearchOptimizer::extrapolateOptimalConfiguration(std::pair<double, double> linear_function, HarmonicDriveParameters &current_drive)
{
    // get current drive values
    double current_offset = current_drive.getOffset();
    double current_slope = current_drive.getSlope();

    // returns the point from the linear func that is closest to the current config
    return StatisticalAnalysis::closest_point_on_line(linear_function, {current_offset, current_slope});
}

// Function to compute and log criteria for a model - to be used for testing purposes only
void GridSearchOptimizer::computeCriteria()
{
    HarmonicsDataHandler harmonics_handler;
    calculator_.reload_and_calc_harmonics(model_handler_.getTempJsonPath(), harmonics_handler);

    for (int i = 1; i <= 10; i++)
    {
        // Evaluate the criteria
        Logger::info("Evaluating criteria for harmonic B" + std::to_string(i));
        for (auto &criterion : criteria_)
        {
            double value;
            // if the criteria is FittedSlopeObjective, pass more params
            if (criterion->getLabel() == "fitted_slope")
            {
                value = std::dynamic_pointer_cast<FittedSlopeObjective>(criterion)->evaluate(harmonics_handler, i, getMinMagnetEll(), getMaxMagnetEll(), true);
            }
            else
            {
                value = criterion->evaluate(harmonics_handler, i);
            }
            Logger::info_double(criterion->getLabel(), value);
        }
    }
}