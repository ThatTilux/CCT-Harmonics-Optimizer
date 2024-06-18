#include "optimizer_bindings.h"


// counts the number of objective function evaluations
int counter_ = 0;


//TODO TEMP REMOVE
// in the python script, create a proper way to address this
void manually_set_objective(){
    boost::filesystem::path path_model = "./../data/quad_test_all_linear.json";

    ModelHandler handler(path_model);
    ObjectiveFunction obj(handler, CHISQUARE_WEIGHT);

    init_objective_binding(std::make_shared<ObjectiveFunction>(obj));    
}

// Initializes the objective
void init_objective_binding(std::shared_ptr<ObjectiveFunction> pObjective)
{
    ObjectiveManager::getInstance().setObjective(pObjective);
}
// Binding to Python that exposes the ObjectiveFunction::objective_function
double objective_binding(const std::vector<double> &params)
{

    counter_++;
    Logger::info("=== Bayesian Optimization run " + std::to_string(counter_) + " ===");

    // Get the objective
    if(ObjectiveManager::getInstance().getObjective() == nullptr){
        manually_set_objective();
        Logger::debug("Manually set objective");
    }

    std::shared_ptr<ObjectiveFunction> objective = ObjectiveManager::getInstance().getObjective();



    // TODO for simplicity, we assume that B2 is ommitted

    // cast parameters to map
    HarmonicDriveParameterMap param_map;
    param_map["B1"] = HarmonicDriveParameters(params[0], params[1]);
    for (int i = 2; i < params.size(); i = i + 2)
    {
        std::string component = std::to_string(2 + (i / 2));
        param_map["B" + component] = HarmonicDriveParameters(params[i], params[i + 1]);
    }

    // print
    Logger::info("New parameters:");

    for (auto &param : param_map)
    {
        Logger::info(param.first + ": " + to_string(param.second));
    }

    ObjectiveFunction obj = *objective;
    double result = obj.objective_function(param_map);

    Logger::info("=== end of run " + std::to_string(counter_) + " ===");

    return result;
}


PYBIND11_MODULE(optimizer_module, m)
{
    m.def("objective_binding", &objective_binding, "Objective function to optimize");
}
