cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(CCTHarmonicsOptimizer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add include directories
include_directories(include)

# Find and link external libraries
find_package(RatModels 2.017.7 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)

# Get all source files in src directory except main.cpp
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

# Create the main executable
add_executable(main ${CMAKE_SOURCE_DIR}/src/main.cpp ${SOURCES})
target_link_libraries(main PRIVATE Rat::Models armadillo Boost::filesystem)

# Google Test integration
# Download and unpack googletest at configure time
configure_file(googletest-download/CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if(result)
  message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
                RESULT_VARIABLE result
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download)
if(result)
  message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                 ${CMAKE_BINARY_DIR}/googletest-build)

# Get all test files in test directory
file(GLOB TEST_SOURCES "test/*.cpp")

# Enable testing
enable_testing()

# Create the test executable without main.cpp
add_executable(runTests ${SOURCES} ${TEST_SOURCES})
target_link_libraries(runTests PRIVATE gtest_main Rat::Models armadillo Boost::filesystem)

# Add tests
add_test(NAME runTests COMMAND runTests)

# Suppress DartConfiguration.tcl warnings
set(CMAKE_SUPPRESS_REGENERATION true)

# Disable CDash
set(CTEST_USE_LAUNCHERS OFF)
