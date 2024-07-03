"""
Bayesian Optimization Script to minimize the slope of the fitted function for B1.
"""

import sys, os
from skopt import gp_minimize
from skopt.space import Real

# Set the path to the shared library
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/lib'))
sys.path.append(lib_path)

import optimizer_module # type: ignore (IDE does not find the package, but it's there :)


# Handcrafted Param Ranges - only specific to _nob6_alllinear.json
B1_OFFSET_MAX = 0.0025 # [m]
B1_SLOPE_MAX = 0.000025; # [m/coil]

# Define the parameter space
space = [Real(-B1_OFFSET_MAX, B1_OFFSET_MAX, name='B1_offset'),
         Real(-B1_SLOPE_MAX, B1_SLOPE_MAX, name='B1_slope')]


# Objective function
def objective(params):
    value = optimizer_module.objective_binding_minimize_slope(params)
    # error code, most likely due to invalid parameters (e.g. coil overlap)
    if value == -1.0:
        return float('inf')
    return value

# Run Bayesian Optimization (tries to minimize the objective function)
result = gp_minimize(objective, space, n_calls=100000, n_random_starts=100, acq_func="EI")

# results do not need to be exported, they are logged in the objective binding
