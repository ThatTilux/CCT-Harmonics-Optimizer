#ifndef OPTIMIZER_BINDINGS_H
#define OPTIMIZER_BINDINGS_H

#include <vector>
#include "objective_function.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Logger.hh"

namespace py = pybind11;

// Singleton Class to manage the ObjectiveFunction instance
class ObjectiveManager {
public:
    static ObjectiveManager& getInstance() {
        static ObjectiveManager instance;
        return instance;
    }

    void setObjective(std::shared_ptr<ObjectiveFunction> pObjective) {
        objective = pObjective;
    }

    std::shared_ptr<ObjectiveFunction> getObjective() {
        return objective;
    }

private:
    ObjectiveManager() : objective(nullptr) {}
    std::shared_ptr<ObjectiveFunction> objective;

    // Delete copy constructor and assignment operator to prevent copying
    ObjectiveManager(const ObjectiveManager&) = delete;
    ObjectiveManager& operator=(const ObjectiveManager&) = delete;
};

void init_objective_binding(std::shared_ptr<ObjectiveFunction> pObjective);

double objective_binding(const std::vector<double> &params);

#endif // OPTIMIZER_BINDINGS_H
