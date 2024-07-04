#include "objective_function.h"

ObjectiveFunction::ObjectiveFunction(const ModelHandler &model_handler, double weight_chisquared)
    : model_handler_(model_handler),
      json_file_path_(model_handler.getTempJsonPath()),
      calculator_(model_handler.getTempJsonPath()),
      weight_chisquared_(weight_chisquared)
{
    // check that there are only custom harmonics with an 'amplitude' of linear
    HarmonicDriveParameterMap params = model_handler_.getHarmonicDriveValues();
    for (auto &param : params)
    {
        if (!param.second.isOffsetAndSlope())
            throw std::runtime_error("The selected model has one or more custom harmonics with an 'amplitude' value other than 'linear'. This is not supported for this optimizer.");
    }
}

// Objective function for Bayesian Optimization. Incorporates the sum of the bn values (except the main one) and chi-square differences to a fitted linear function
// A lower value is better, 0 is the minimum
// There are 18 input params: offset and slope for all customs harmonics B1...B10 except for the main one (or 9 with just the offsets incase the bn optimizer is used)
double ObjectiveFunction::objective_function(HarmonicDriveParameterMap &params)
{
    // TODO MAKE THIS MORE MODULAR

    // apply all parameters
    model_handler_.apply_params(params);

    // if the bn optimizer should be used, apply it
    if (USE_BN_OPTIMIZER_IN_CHISQUARED)
    {
        //TODO TAKE CARE OF THIS
    }

    // do the computation
    HarmonicsHandler harmonics_handler;
    calculator_.reload_and_calc(json_file_path_, harmonics_handler);
    std::vector<double> current_bn_values = harmonics_handler.get_bn();

    // print bn values
    print_vector(current_bn_values, "bn");

    // get the sum of all absolute bns except for the main one
    double sum_bn = std::accumulate(current_bn_values.begin(), current_bn_values.end(), 0.0, [](double a, double b)
                                    { return a + std::abs(b); }) -
                    std::abs(current_bn_values[MAIN_COMPONENT - 1]);

    // distance of the main bn value to 10,000
    double distance_main_10000 = 10000 - std::abs(current_bn_values[MAIN_COMPONENT - 1]);

    // sum up all chi squared values except for the main one
    double sum_chisquared = 0;
    Logger::info("chiSquared values:");
    for (int i = 1; i <= 10; i++)
    {
        if (i != MAIN_COMPONENT)
        {
            double value = chiSquared(harmonics_handler, i);
            sum_chisquared += value;
            Logger::info("chiSquared[" + std::to_string(i) + "]: " + std::to_string(value));
        }
    }

    Logger::info("bn objective value: " + std::to_string(sum_bn));
    Logger::info("distance of main bn to 10,000: " + std::to_string(distance_main_10000));
    Logger::info("chiSquared objective value: " + std::to_string(sum_chisquared) + ", weighted: " + std::to_string(sum_chisquared * weight_chisquared_));

    // compute the objective function
    double objective_value;
    if (USE_BN_OPTIMIZER_IN_CHISQUARED)
    {
        Logger::info("Only including chiSquared for the objective function");
        objective_value = sum_chisquared;
    }
    else
    {
        objective_value = sum_bn + distance_main_10000 + weight_chisquared_ * sum_chisquared;
    }

    Logger::info("objective function value: " + std::to_string(objective_value));

    return objective_value;
}

// Function to transform the drive values to be applicable for the RAT model
void transform_drive_values(double &offset, double &slope){
    // linear regression gives values in m, RAT needs mm
    offset = offset / 1000;
    slope = slope / 1000;
}

// TODO this is a temp function for testing: compute chi squared for one component and set offset, slope based on those. Returns error code
int ObjectiveFunction::chiSquaredOptimizer(int component, double scaling_factor, bool temp_do_bn_optimizer)
{
    // run harmonics calculation
    HarmonicsHandler harmonics_handler;
    calculator_.reload_and_calc(json_file_path_, harmonics_handler);
    std::vector<double> current_bn_values = harmonics_handler.get_bn();

    // print bn values
    print_vector(current_bn_values, "bn");

    // pair that the chisquared will store the offset and slope in
    std::pair<double, double> fitted;

    // run chi square computation for set component
    double value = chiSquared(harmonics_handler, component, &fitted);

    Logger::debug("Chi squared value for B" + std::to_string(component) + ": " + std::to_string(value));

    // const std::vector<double> SCALING_FACTORS = {0.1, 0.01, 0.001};
    
    // transform values to be applicable for drives
    transform_drive_values(fitted.first, fitted.second);

    // apply values
    Logger::info("Now applying the fitted values for B" + std::to_string(component) + ": offset = " + std::to_string(fitted.first) + ", slope = " + std::to_string(fitted.second));
    model_handler_.setHarmonicDriveValue("B" + std::to_string(component), HarmonicDriveParameters(fitted.first / 1000, fitted.second));

    return 0;
}

// TODO clean this up - e.g., put into other obj func class
// Function that fits a linear function to a Bn function and returns the absolute slope
double ObjectiveFunction::objective_function_slope(HarmonicDriveParameterMap &params){

    // a fitted abs slope 0.0015 is considered good
    // a abs bn value of 10 is considered good
    // set the weight so that these two are equally good 
    const double SLOPE_WEIGHT = 10/0.0015;

    // apply all parameters
    model_handler_.apply_params(params);

    // do the computation
    HarmonicsHandler harmonics_handler;
    calculator_.reload_and_calc(json_file_path_, harmonics_handler);

    // get bn values
    std::vector<double> current_bn_values = harmonics_handler.get_bn();

    double bn = current_bn_values[0];

    Logger::info("B1 bn value: " + std::to_string(bn));

    // pair that the chisquared will store the offset and slope in
    std::pair<double, double> fitted;

    // run chi square computation to get fit
    double value = chiSquared(harmonics_handler, 1, &fitted); //TODO hardcoded B1 here

    double slope = std::abs(fitted.second);
    double weighted_slope = slope * SLOPE_WEIGHT;

    Logger::info("Absolute slope value: " + std::to_string(slope));
    Logger::info("Weighted slope value: " + std::to_string(weighted_slope));

    double obj_value = weighted_slope + bn;

    Logger::info("Objective function value: " + std::to_string(obj_value));

    //return slope
    return slope;
}

// Function to compute chi-squared distance between (1) a function described by the points vector and (2) a linear function described by slope, intercept
double computeChiSquared(const std::vector<std::pair<double, double>> &points, double slope, double intercept, double variance_y)
{
    double chi_squared = 0;
    for (const auto &point : points)
    {
        double y_fit = slope * point.first + intercept;
        double residual = point.second - y_fit;
        chi_squared += (residual * residual) / variance_y;
    }
    return chi_squared;
}


// Funtion to apply transformations to the ell,Bn data for the chisquared computation
void apply_chisquared_transformation(std::vector<std::pair<double, double>> &Bn_data){
    // remove all the pairs where the ell value is not inside the set bounds
    Bn_data.erase(std::remove_if(Bn_data.begin(), Bn_data.end(), [](const std::pair<double, double> &pair)
                              { return pair.first < MAG_START_POS || pair.first > MAG_END_POS; }),
               Bn_data.end());

    //make sure there are at least 2 points
    if (Bn_data.size() < 2)
    {
        throw std::runtime_error("Not enough points for chisquared computation.");
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


// Function to compute the chi square for a Bn component. A linear function will be fitted to the data and chi square will be computed between that and the data. If fitted is set to a pair, the fitted offset and slope will be stored there
double chiSquared(HarmonicsHandler &harmonics_handler, int component, std::pair<double, double> *fitted)
{
    // get the Bn and ell
    std::vector<std::pair<double, double>> points = harmonics_handler.get_Bn(component);

    // apply transformations to the data
    apply_chisquared_transformation(points);

    // export the points
    //export_data_to_csv(points, "./Bn/Bn_component_" + std::to_string(component) + ".csv");

    // extract the Bn values
    std::vector<double> Bn;
    for (const auto& point : points)
    {
        Bn.push_back(point.second);
    }

    // get the variance of Bn
    double variance = computeVariance(Bn);

    // fit a linear function
    auto [slope, intercept] = AbstractOptimizer::linearRegression(points);

    // fill fitted if needed
    if (fitted != nullptr)
    {
        fitted->first = intercept;
        fitted->second = slope;
    }

    Logger::debug("Fitted linear function for component B" + std::to_string(component) + ": y = " + std::to_string(slope) + "x + " + std::to_string(intercept));

    // compute chi squared between the function and the original data
    double chi_squared = computeChiSquared(points, slope, intercept, variance);

    return chi_squared;
}



// Function to compute the variance of y values with Bessel's correction
double computeVariance(const std::vector<double> &y)
{
    double mean = std::accumulate(y.begin(), y.end(), 0.0) / y.size();
    double variance = 0;
    for (const auto &value : y)
    {
        variance += (value - mean) * (value - mean);
    }
    variance /= (y.size() - 1); // Bessel's correction
    return variance;
}