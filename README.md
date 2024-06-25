# CCT Harmonics Optimizer

This software is designed to automatically optimize custom CCT (canted-cosine-theta) harmonic drive parameters for CCT magnets. The optimizer adjusts the scaling function parameters of the custom harmonics to create a magnetic field where only the main B component has a bn value of 10,000, while the other components have bn values of 0 (within a user-specified margin). This project is part of the FCC-ee HTS4 research project at CERN.

## Overview

This software utilizes the open-source RAT Library. For more information about the RAT Library, visit the [RAT Library website](https://rat-gui.com/library.html).

## Installation

**Note: This software only works on Linux.**

TODO: 
- add libgtest-dev requirement (apt)
- and python3-dev, scikit-optimize, pybind11 requirement (with pip)
- add sudo apt-get install pybind11-dev requirement
- add spdlog requirement (with vcpkg or clone + install)
- add logging explanation

1. Follow the [RAT documentation](https://gitlab.com/Project-Rat/rat-documentation) to install the RAT library. This may take some time.

2. Clone this repository and build the software:
    ```sh
    git clone <repository_url>
    cd <repository_directory>
    mkdir build
    cd build
    cmake ..
    make
    ```

3. Run all tests:
    ```sh
    ./bin/runTests
    ```

4. To run the program:
    ```sh
    ./bin/main
    ```

## Usage

**TODO**: only linear custom ccts allowed, otherwise change? Also: update naming convention of custom harmonics

The optimizer requires a JSON file of a CCT magnet, created by the [RAT-GUI software](https://rat-gui.com/index.html). Ensure that the JSON file meets the following criteria:

- The JSON file should contain a harmonics calculation in the calculation tree. The optimizer will use the first harmonics calculation in the tree.
- The JSON file should include at least one custom CCT harmonic in the model tree. These harmonics will be optimized by the software.
    - The harmonics should have names that start with 'B' (e.g., B1 for dipole harmonics, B10 for decapole harmonics).
    - The main harmonic, which needs a bn value of 10,000, should be named differently to prevent optimization by the software.
    - Custom CCT harmonics must have an 'amplitude' of 'constant' or 'linear'. For 'constant', the 'constant' parameter will be optimized. For 'linear', the 'slope' parameter will be optimized.
    - All custom harmonics with the same number of poles should have the same name and scaling function parameters, ensuring they are optimized together.


Place the JSON file in the `data` directory. Run the program using the command specified above (`./bin/main`). You will be prompted to enter the maximum absolute bn value. For example, entering `0.1` will optimize the harmonics to achieve bn values within the range of -0.1 to 0.1.
Caution is advised for values below 0.1 as runtime will explode.

### Example

An example model `cct.json` can be found in the `examples` directory. To test this software, follow these steps:

1. Place the `cct.json` file in the `data` directory.
2. Run the program:
    ```sh
    ./bin/main
    ```
3. Follow the prompts to select the correct JSON file and enter 0.1 (or higher) as the maximum absolute bn value.

The program will terminate after a some minutes, providing the optimal parameters.

## Optimizer

The optimizer leverages the almost-linear relationship between the scaling function of BX and the corresponding bn value of BX. The process is as follows:

1. The software performs harmonic calculations for at least two different values of the respective scaling function parameter (constant or slope).
2. A linear regression is applied to extrapolate the optimal parameter value that results in a bn value of 0.
3. The optimization is done iteratively in rounds since the relationship is not completely linear and changing the scaling function of one harmonic affects other harmonics.

The optimizer continues to adjust parameters until the desired result is achieved. It only optimizes the bn values of those BX components that have a corresponding custom harmonic named BX (which should have X poles).

## Author

Ole Kuhlmann  
Email: [tim.ole.kuhlmann@cern.ch](mailto:tim.ole.kuhlmann@cern.ch)  
GitHub: [ThatTilux](https://github.com/ThatTilux)

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
