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
    // TODO
}

// Function to start the optimizer
void GridSearchOptimizer::optimize()
{
    Logger::info("=== Starting grid search optimizer ===");

    // flag that all bn values are below a certain threshold
    bool allHarmonicsBelowThreshold = false;

    // run grid searches until all harmonics are below a threshold
    while (!allHarmonicsBelowThreshold)
    {
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

            // run the grid search
            std::vector<GridSearchResult> results;
            runGridSearch(i, results);

            // Extrapolate the optimal configuration
            // TODO; for now, export the results to csv
            export_grid_search_results_to_csv(results, "./grid_search/grid_search_results_B" + std::to_string(i) + ".csv");
        }

        // check if all are below a threshold
        allHarmonicsBelowThreshold = areAllHarmonicsBelowThreshold(GRID_BN_THRESHOLD);

        // TODO remove once extraploation has happened
        allHarmonicsBelowThreshold = true;
    }

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
