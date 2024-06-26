import sys, os
from skopt import gp_minimize
from skopt.space import Real

# Set the path to the shared library
lib_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../build/lib'))
sys.path.append(lib_path)

import optimizer_module # type: ignore (IDE does not find the package, but it's there :)


# Param Ranges
B1_OFFSET_MAX = 0.05 # 5 mm
B4_OFFSET_MAX = 0.015 # 1.5 mm
B5_OFFSET_MAX = 0.01 # 1 mm
SLOPE_MAX = 0.0001 # 1e-04 m/coil
SLOPE_MIN = -SLOPE_MAX

# Define the parameter space
# TODO REMOVE MANUAL B2 OMITTANCE
# TODO LET USER DECIDE IF ONLY OFFSETS
space = [Real(-B1_OFFSET_MAX, B1_OFFSET_MAX, name='B1; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B1; slope'),
         Real(-B1_OFFSET_MAX, B1_OFFSET_MAX, name='B3; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B3; slope'),
         Real(-B4_OFFSET_MAX, B4_OFFSET_MAX, name='B4; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B4; slope'),
         Real(-B5_OFFSET_MAX, B5_OFFSET_MAX, name='B5; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B5; slope'),
         Real(-B5_OFFSET_MAX, B5_OFFSET_MAX, name='B6; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B6; slope'),
         Real(-B5_OFFSET_MAX, B5_OFFSET_MAX, name='B7; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B7; slope'),
         Real(-B5_OFFSET_MAX, B5_OFFSET_MAX, name='B8; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B8; slope'),
         Real(-B5_OFFSET_MAX, B5_OFFSET_MAX, name='B9; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B9; slope'),
         Real(-B5_OFFSET_MAX, B5_OFFSET_MAX, name='B10; offset'),
         #Real(SLOPE_MIN, SLOPE_MAX, name='B10; slope'),
        ]


# Objective function
def objective(params):
    value = optimizer_module.objective_binding(params)
    # error code
    if value == -1.0:
        raise RuntimeError('objective binding returned error code')
    return value

# Run Bayesian Optimization (tries to minimize the objective function)
result = gp_minimize(objective, space, n_calls=50, n_random_starts=10, acq_func="EI")

# results do not need to be exported, they are logged in the objective binding
