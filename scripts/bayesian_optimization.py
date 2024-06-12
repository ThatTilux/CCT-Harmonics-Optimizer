import sys, os
from skopt import gp_minimize
from skopt.space import Real

# Set the path to the shared library
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/lib'))
sys.path.append(lib_path)

import optimizer_module # type: ignore (IDE does not find the package, but it's there :)


# Param Ranges
OFFSET_MAX = 0.05 # 5 mm
OFFSET_MIN = -OFFSET_MAX 
SLOPE_MAX = 0.0001 # 1e-04 m/coil
SLOPE_MIN = -SLOPE_MAX

# Define the parameter space
# TODO REMOVE MANUAL B2 OMITTANCE
space = [Real(OFFSET_MIN, OFFSET_MAX, name='B1; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B1; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B3; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B3; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B4; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B4; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B5; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B5; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B6; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B6; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B7; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B7; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B8; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B8; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B9; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B9; slope'),
         Real(OFFSET_MIN, OFFSET_MAX, name='B10; offset'),
         Real(SLOPE_MIN, SLOPE_MAX, name='B10; slope'),
        ]


# Objective function
def objective(params):
    value = optimizer_module.objective_binding(params)
    # error code
    if value == -1.0:
        raise RuntimeError('objective binding returned error code')
    return value

# Run Bayesian Optimization
result = gp_minimize(objective, space, n_calls=50, n_random_starts=10, acq_func="EI")

# Print the results
print("Best score:", result.fun)
print("Best parameters:", result.x)

# Save results for later use in C++
with open('optimization_results.txt', 'w') as f:
    f.write(f"Best score: {result.fun}\n")
    f.write("Best parameters: " + " ".join(map(str, result.x)) + "\n")
