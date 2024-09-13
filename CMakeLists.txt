cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
project(CCTHarmonicsOptimizer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add include directories
include_directories(include)

# Ensure Python library path is included
link_directories(${Python3_LIBRARY_DIRS})

# Find and link external libraries
find_package(RatModels 2.017.7 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Eigen3 REQUIRED)


# find ZLIB
if(POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif()

find_package(ZLIB REQUIRED)

# Fetch CCTools at configure time
include(FetchContent)

FetchContent_Declare(
  CCTools
  GIT_REPOSITORY https://github.com/ThatTilux/CCTools.git
  GIT_TAG        main
)

FetchContent_MakeAvailable(CCTools)


# Get all source files in src directory
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")


# Create the main executable
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp ${SOURCES})
target_include_directories(main PRIVATE ${EIGEN3_INCLUDE_DIR})
target_link_libraries(main PRIVATE Rat::Models CCTools armadillo Boost::filesystem)


# Get all test files in test directory
file(GLOB TEST_SOURCES "test/*.cpp")

# Enable testing
enable_testing()

# Create the test executable
add_executable(runTests ${SOURCES} ${TEST_SOURCES})
target_include_directories(runTests PRIVATE ${EIGEN3_INCLUDE_DIR})
target_link_libraries(runTests PRIVATE gtest_main Rat::Models CCTools armadillo Boost::filesystem)

# Add tests
add_test(NAME runTests COMMAND runTests)

# Suppress DartConfiguration.tcl warnings
set(CMAKE_SUPPRESS_REGENERATION true)

# Disable CDash
set(CTEST_USE_LAUNCHERS OFF)