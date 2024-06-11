#include "optimizer_bindings.h"

thread_local std::shared_ptr<ObjectiveFunction> thread_local_objective = nullptr;


//TODO TEMP REMOVE
// in the python script, create a proper way to address this
void manually_set_objective(){
    boost::filesystem::path path_model = "./../data/quad_test_all_linear.json";
    double chiSquare_weight = 0.01;

    ModelHandler handler(path_model);
    ObjectiveFunction obj(handler, chiSquare_weight);

    init_objective_binding(std::make_shared<ObjectiveFunction>(obj));    
}

// Initializes the objective
void init_objective_binding(std::shared_ptr<ObjectiveFunction> pObjective)
{
    ObjectiveManager::getInstance().setObjective(pObjective);
    std::cout << "Objective initialized: " << ObjectiveManager::getInstance().getObjective().get() << std::endl;;
}
// Binding to Python that exposes the ObjectiveFunction::objective_function
double objective_binding(const std::vector<double> &params)
{

    // Get the objective
    if(ObjectiveManager::getInstance().getObjective() == nullptr){
        manually_set_objective();
        std::cout << "Manually set objective" << std::endl;
    }

    std::shared_ptr<ObjectiveFunction> objective = ObjectiveManager::getInstance().getObjective();


    // TODO REMOVE PRINTS
    // Debug print
    if (!objective) {
        std::cerr << "Error: objective is not initialized" << std::endl;
        return -1.0;
    }
    
    std::cout << "Objective address: " << objective.get() << std::endl;


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
    std::cout << "Now computing objective function for the parameters:" << std::endl;

    for (auto &param : param_map)
    {
        std::cout << param.first << ": " << to_string(param.second) << std::endl;
    }

    // TODO REMOVE
    // Debug print
    std::cout << "Dereferencing objective: " << objective.get() << std::endl;

    ObjectiveFunction obj = *objective;
    double result = obj.objective_function(param_map);

    std::cout << "Result: " << result << std::endl;

    return result;
}

// TODO TEMP REMOVE
void check_objective_state() {
    py::gil_scoped_acquire acquire;  // Ensure GIL is acquired
    auto objective = ObjectiveManager::getInstance().getObjective();
    std::cout << "Checking objective state: " << objective.get() << std::endl;
}

PYBIND11_MODULE(optimizer_module, m)
{
    m.def("objective_binding", &objective_binding, "Objective function to optimize");
    //TODO TEMP REMOVE
    m.def("check_objective_state", &check_objective_state, "Check the state of the global objective");
}
