cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(RatExtension VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Add source files
set(SOURCES src/main.cpp)

# Add include directories
include_directories(include)

# Find and link external libraries
find_package(RatModels 2.017.7 REQUIRED)
find_package(Armadillo REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem)

# Create the executable
add_executable(rat_extension ${SOURCES})

# Link libraries
target_link_libraries(rat_extension PRIVATE Rat::Models armadillo Boost::filesystem)
