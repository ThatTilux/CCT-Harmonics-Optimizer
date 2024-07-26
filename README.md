# CCT Harmonics Optimizer

This software is designed to automatically optimize custom CCT (canted-cosine-theta) harmonic drive parameters for CCT magnets. The optimizer adjusts the scaling function parameters of the custom harmonics to create a magnetic field where only the main B component has a bn value of 10,000, while the other components have bn values of 0 (within a user-specified margin). This project is part of the FCC-ee HTS4 research project at CERN.
TODO revise

## Overview

This software utilizes the open-source RAT Library. For more information about the RAT Library, visit the [RAT Library website](https://rat-gui.com/library.html).

## Installation

**Note: This software only works on Linux.**

### Prerequisites

Please follow the instructions below to install all necessary dependencies.

#### System Packages

1. **Google Test (gtest) Library**:
   - Google Test is a framework for C++ tests.
   ```sh
   sudo apt-get install libgtest-dev
   ```

2. **spdlog**:
   - spdlog is a fast C++ logging library.
   You can install `spdlog` using by cloning the repository and installing it manually:
   ```sh
   git clone https://github.com/gabime/spdlog.git
   cd spdlog
   mkdir build && cd build
   cmake ..
   make -j
   sudo make install
   ```

3. **Eigen3**
    - Eigen3 is a library used for linear algebra computations
    ```sh
    sudo apt-get install libeigen3-dev
    ```

#### RAT-Library
Follow the [RAT documentation](https://gitlab.com/Project-Rat/rat-documentation) to install the RAT library. This may take a while.
- When a CUDA compatible GPU is available, make sure to follow the steps for installing CUDA. This is highly recommended as it reduces the runtime of this software by some orders of magnitude.
- When using CUDA with WSL, it is advised to increase the allocated memory for WSL, see [here](https://geronimo-bergk.medium.com/optimizing-wsl2-performance-setting-memory-and-cpu-allocation-on-windows-513eba7b6086).



### Building and Running the Software

1. Clone this repository and build the software:
    ```sh
    git clone <repository_url>
    cd <repository_directory>
    mkdir build
    cd build
    cmake ..
    make
    ```

2. Run all tests:
    ```sh
    ./bin/runTests
    ```

3. To run the program:
    ```sh
    ./bin/main
    ```

## Theoretical Background

The magnetic field produced by CCT magnets can be described by a series of harmonics, denoted as B1, B2, ..., B10. 
In this context, B1 represents the dipole field, B2 represents the quadrupole field, and so on up to the decapole field (B10).

When modeling CCT magnets using the RAT Library, custom harmonics can be defined to fine-tune the magnetic field. These custom harmonics allow for precise control over the magnetic field distribution. 

Each custom harmonic can be characterized by its scaling function.
Custom harmonics in the CCT model have a specific number of poles (X) and are associated primarily with the harmonic BX. When simulating the magnet and computing harmonics, we are interested in 2 properties for each harmonic component:

- **Bn Curve**: The Bn curve represents the magnitude of the harmonic along the length of the magnet, measured in Tesla (T).
- **bn Value**: The bn value is the integral of the Bn curve over the length of the magnet. It indicates the overall magnitude of the harmonic component.


## Optimizers

## Usage

**TODO**: only linear custom ccts allowed, otherwise change? Also: update naming convention of custom harmonics; Also: include second optimizer in this

The optimizer requires a JSON file of a CCT magnet, created by the [RAT-GUI software](https://rat-gui.com/index.html). Ensure that the JSON file meets the following criteria:

- The JSON file should contain a harmonics calculation in the calculation tree. The optimizer will use the first harmonics calculation in the tree.
- The JSON file should include at least one custom CCT harmonic in the model tree. These harmonics will be optimized by the software.
    - The harmonics should have names that start with 'B' (e.g., B1 for dipole harmonics, B10 for decapole harmonics).
    - The main harmonic, which needs a bn value of 10,000, should be named differently to prevent optimization by the software.
    - Custom CCT harmonics must have an 'amplitude' of 'constant' or 'linear'. For 'constant', the 'constant' parameter will be optimized. For 'linear', the 'slope' parameter will be optimized.
    - All custom harmonics with the same number of poles should have the same name and scaling function parameters, ensuring they are optimized together.


Place the JSON file in the `data` directory. Run the program using the command specified above (`./bin/main`). You will be prompted to enter the maximum absolute bn value. For example, entering `0.1` will optimize the harmonics to achieve bn values within the range of -0.1 to 0.1.
Caution is advised for values below 0.1 as runtime will explode.

Detailed logs are saved in the `logs` directory.

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
