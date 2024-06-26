#include "optimizer_bindings.h"

// counts the number of objective function evaluations
int counter_ = 0;

// Function to initialize the objective function
void init_objective()
{
    boost::filesystem::path path_model = selectJsonFile();

    ModelHandler handler(path_model);
    ObjectiveFunction obj(handler, CHISQUARE_WEIGHT);

    init_objective_binding(std::make_shared<ObjectiveFunction>(obj));
}

// Initializes the objective
void init_objective_binding(std::shared_ptr<ObjectiveFunction> pObjective)
{
    ObjectiveManager::getInstance().setObjective(pObjective);
}

// function to cast a vector of parameters to a map
void cast_params_to_map(const std::vector<double> &params, HarmonicDriveParameterMap &param_map)
{
    // if there are 9 parameters, these are only offset values. if there are 18, the offset is also included
    assert(params.size() == 9 || params.size() == 18);

    if (params.size() == 18)
    {
        param_map["B1"] = HarmonicDriveParameters(params[0], params[1]);
        for (int i = 2; i < params.size(); i = i + 2)
        {
            std::string component = std::to_string(2 + (i / 2));
            param_map["B" + component] = HarmonicDriveParameters(params[i], params[i + 1]);
        }
    }
    else if (params.size() == 9)
    {
        param_map["B1"] = HarmonicDriveParameters(params[0], HarmonicDriveParameterType::Offset);
        for (int i = 1; i < params.size(); i++)
        {
            // TODO for simplicity, we assume that B2 is omitted -> params[0] is B1, params[1] is B3, etc.
            std::string component = std::to_string(i + 2);
            param_map["B" + component] = HarmonicDriveParameters(params[i], HarmonicDriveParameterType::Offset);
        }
    }
}

// Binding to Python that exposes the ObjectiveFunction::objective_function
double objective_binding(const std::vector<double> &params)
{
    Logger::log_timestamp("Objective binding called from Python");

    counter_++;
    Logger::info("=== Bayesian Optimization run " + std::to_string(counter_) + " ===");

    // Get the objective
    if (ObjectiveManager::getInstance().getObjective() == nullptr)
    {
        init_objective();
        Logger::debug("Initialized objective");
    }

    std::shared_ptr<ObjectiveFunction> objective = ObjectiveManager::getInstance().getObjective();

    // TODO for simplicity, we assume that B2 is ommitted

    // cast parameters to map
    HarmonicDriveParameterMap param_map;
    cast_params_to_map(params, param_map);

    // print
    Logger::info("New parameters:");
    for (auto &param : param_map)
    {
        Logger::info(param.first + ": " + to_string(param.second));
    }

    ObjectiveFunction obj = *objective;
    Logger::log_timestamp("Calling objective function");
    double result = obj.objective_function(param_map);
    Logger::log_timestamp("Objective function finished");

    Logger::info("=== end of run " + std::to_string(counter_) + " ===");

    Logger::log_timestamp("Objective binding finished");

    return result;
}

PYBIND11_MODULE(optimizer_module, m)
{
    m.def("objective_binding", &objective_binding, "Objective function to optimize");
}
