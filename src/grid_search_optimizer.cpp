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
    initParamRanges();
    initCriteria();
    estimateTimePerComputation();
    initGranularities();
}

// Constructor to be used for no user interaction.
GridSearchOptimizer::GridSearchOptimizer(ModelHandler &model_handler) : AbstractOptimizer()
{
    // TODO
}

// Function to initialize the parameter ranges for the grid search
void GridSearchOptimizer::initParamRanges()
{
    // TODO infer these automatically from the model

    // initialize param_ranges_ with the correct size
    param_ranges_.resize(10);

    // B1 for the quad_double_nob6_alllinear
    param_ranges_[0] = {{-0.0025, 0.0025}, {-0.000025, 0.000025}};

    // TODO add other harmonics
    param_ranges_[1] = {{0, 1}, {0, 1}};
    param_ranges_[2] = {{0, 1}, {0, 1}};
    param_ranges_[3] = {{0, 1}, {0, 1}};
    param_ranges_[4] = {{0, 1}, {0, 1}};
    param_ranges_[5] = {{0, 1}, {0, 1}};
    param_ranges_[6] = {{0, 1}, {0, 1}};
    param_ranges_[7] = {{0, 1}, {0, 1}};
    param_ranges_[8] = {{0, 1}, {0, 1}};
    param_ranges_[9] = {{0, 1}, {0, 1}};
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
void GridSearchOptimizer::initGranularities()
{
    // for each harmonic, initialize granularities
    for (int i = 1; i <= 10; i++)
    {
        auto [offset_range, slope_range] = getParamRange(i);
        std::pair<double, double> granularities = computeGranularities(offset_range, slope_range, TIME_BUDGET_GRID_SEARCH, time_per_calc_);
        granularities_.push_back(granularities);
        Logger::info("Granularities for harmonic B" + std::to_string(i) + ": Offset: " + std::to_string(granularities.first) + ", Slope: " + std::to_string(granularities.second));
    }
}

// Function to compute granularities given a time budget. Granularity of offset is granularity of slope * 10. Format: {offset_granularity, slope_granularity}
std::pair<double, double> GridSearchOptimizer::computeGranularities(std::pair<double, double> offset_range,
                                                                    std::pair<double, double> slope_range,
                                                                    double time_budget_minutes,
                                                                    double time_per_step_seconds)
{
    double time_budget_seconds = time_budget_minutes * 60.0;
    double slope_range_size = slope_range.second - slope_range.first;
    double offset_range_size = offset_range.second - offset_range.first;

    // Calculate total steps allowed within the time budget
    double total_steps = time_budget_seconds / time_per_step_seconds;

    // Calculate the number of slope steps
    double slope_steps = std::sqrt(total_steps / 10.0);

    // Calculate the granularities
    double slope_granularity = slope_range_size / std::max(slope_steps, 1.0); // Ensure slope_steps is at least 1
    double offset_granularity = slope_granularity * 10.0;

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
        HarmonicsHandler handler;
        calculator_.reload_and_calc(model_handler_.getTempJsonPath(), handler);
    }

    // End time
    auto end = std::chrono::high_resolution_clock::now();

    // Compute the time
    std::chrono::duration<double> elapsed = end - start;
    double time_per_computation = elapsed.count() / num_computations;

    Logger::info("Estimated time per computation: " + std::to_string(time_per_computation) + " seconds");

    time_per_calc_ = time_per_computation;
}

int GridSearchOptimizer::getNumberOfSteps(std::pair<double, double> offset_range, std::pair<double, double> slope_range, double offset_granularity, double slope_granularity)
{
    int offset_steps = static_cast<int>((offset_range.second - offset_range.first) / offset_granularity) + 1;
    int slope_steps = static_cast<int>((slope_range.second - slope_range.first) / slope_granularity) + 1;
    return offset_steps * slope_steps;
}

void GridSearchOptimizer::logResults()
{
    Logger::info("=== Grid Search Optimizer has finished ===");
    print_vector(current_bn_values_, "bn");
}

// Function to start the optimizer
void GridSearchOptimizer::optimize()
{
    Logger::info("=== Starting grid search optimizer ===");
    Logger::info("Using the following criteria:");
    for (int i = 0; i < criteria_.size(); i++)
    {
        Logger::info("Criterion " + std::to_string(i) + ": " + criteria_[i]->getLabel());
    }

    // flag that all bn values are below a certain threshold
    bool allHarmonicsBelowThreshold;

    // run grid searches until all harmonics are below a threshold
    do
    {
        allHarmonicsBelowThreshold = true;

        // run grid searches
        // TODO do for all components
        for (int i = 1; i <= 1; i++)
        {
            // check if the bn value is good enough already
            if (current_bn_values_.size() >= i && std::abs(current_bn_values_[i - 1]) < GRID_BN_THRESHOLD)
            {
                Logger::info("== Harmonic B" + std::to_string(i) + " is already below the threshold. Skipping. ==");
                continue;
            }
            else
            {
                // there is at least one harmonic not below the threshold
                allHarmonicsBelowThreshold = false;
            }

            // run the grid search
            std::vector<GridSearchResult> results;
            runGridSearch(i, results);

            // export the results to csv
            export_grid_search_results_to_csv(results, "./grid_search/grid_search_results_B" + std::to_string(i) + ".csv");

            // Extrapolate the optimal configuration
            auto [new_offset, new_slope] = extrapolateOptimalConfiguration(results);
            Logger::info("Extrapolated optimal configuration for harmonic B" + std::to_string(i) + ": Offset: " + std::to_string(new_offset) + ", Slope: " + std::to_string(new_slope) + ".");

            // Update the model with the new configuration
            HarmonicDriveParameterMap new_config;
            new_config["B" + std::to_string(i)] = HarmonicDriveParameters(new_offset, new_slope);
            model_handler_.apply_params(new_config);

            // Recompute bn 
            HarmonicsHandler handler;
            calculator_.reload_and_calc(model_handler_.getTempJsonPath(), handler);
            current_bn_values_ = handler.get_bn();
        }

        // TODO remove once extraploation has happened
        allHarmonicsBelowThreshold = true;
    } while (!allHarmonicsBelowThreshold);

    // Start next phase: Do fine-granular grid searches for all harmonics
    // TODO
}

void GridSearchOptimizer::runGridSearch(int component, std::vector<GridSearchResult> &results)
{
    // get the parameter range for the current harmonic
    auto [offset_range, slope_range] = getParamRange(component);

    // run grid search
    std::pair<double, double> granularities = granularities_[component - 1];
    int steps = getNumberOfSteps(offset_range, slope_range, granularities.first, granularities.second);
    GridSearch grid_search(model_handler_, calculator_, component, offset_range, slope_range, granularities.first, granularities.second, results, criteria_, time_per_calc_, steps);
}

// Function to extrapolate the optimal configuration from the grid search results. The optimal offset, slope configuration is the one that minimizes all objectives (criteria).
std::pair<double, double> GridSearchOptimizer::extrapolateOptimalConfiguration(std::vector<GridSearchResult> &results){
    // make sure there are at least 2 criteria
    if (criteria_.size() < 2){
        throw std::runtime_error("At least 2 criteria are needed for extrapolation.");
    }
    
    // linear functions for all criteria
    std::vector<std::pair<double, double>> linear_functions;
    
    for (int i = 0; i < criteria_.size(); i++){
        // Model each data as a 2-d plane in the [offset, slope, criteria] space. Plane: z=ax+by+c
        auto [a, b, c] = StatisticalAnalysis::fitPlaneToData(results, i);
        Logger::info("Plane coefficients for criterion " + std::to_string(i) + ": a=" + std::to_string(a) + ", b=" + std::to_string(b) + ", c=" + std::to_string(c));

        // From each plane, extract the linear function where the plane has the z value 0
        auto [offset, slope] = StatisticalAnalysis::planeToLinearFunction(a, b, c);
        Logger::info("Linear function for criterion " + std::to_string(i) + ": Offset=" + std::to_string(offset) + ", Slope=" + std::to_string(slope));

        linear_functions.push_back({offset, slope});
    }

    if (criteria_.size() > 3){
        throw std::runtime_error("Extrapolation for more than 3 criteria is not implemented.");
    }

    // Calculate the intersection of the linear functions
    auto intersection = StatisticalAnalysis::findIntersection(linear_functions[0], linear_functions[1]);
    
    // Check if they intersect
    if(!intersection){
        throw std::runtime_error("No intersection found for the linear functions.");
    }

    Logger::info("Intersection of the linear functions: Offset=" + std::to_string(intersection->first) + ", Slope=" + std::to_string(intersection->second));

    // Return the new values
    return *intersection;
}
