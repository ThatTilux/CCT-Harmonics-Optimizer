#ifndef HARMONIC_DRIVE_PARAMETER_H
#define HARMONIC_DRIVE_PARAMETER_H

#include <variant>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include "model_handler.h"

// Forward declaration of the HarmonicDriveParameterType enum
enum class HarmonicDriveParameterType;

// Harmonic Drive parameters for custom cct harmonics with 'amplitude' = linear | constant
struct HarmonicDriveParameters {
    // Enum to track the type of parameter
    enum class Type {
        Offset,
        Slope,
        Constant,
        OffsetAndSlope,
        Undefined
    };

    // Constructors for different parameter types
    HarmonicDriveParameters();
    HarmonicDriveParameters(double offset, double slope);
    HarmonicDriveParameters(double value, HarmonicDriveParameterType t);

    // Methods to GET values
    double getConstant() const;
    double getOffset() const;
    double getSlope() const;
    double get(HarmonicDriveParameterType type) const;

    // Methods to check the type
    bool isConstant() const;
    bool isOffset() const;
    bool isSlope() const;
    bool isOffsetAndSlope() const;
    bool isType(HarmonicDriveParameterType t) const;

    // Method to SET values
    void setValue(double value, HarmonicDriveParameterType t);

    // Equality operator
    bool operator==(const HarmonicDriveParameters& other) const;

private:
    Type type;
    double offset;
    double slope;
    double constant;
};

// to_string function for HarmonicDriveParameters
std::string to_string(const HarmonicDriveParameters &params);

// Typedefinition for the unordered map
using HarmonicDriveParameterMap = std::unordered_map<std::string, HarmonicDriveParameters>;

#endif // HARMONIC_DRIVE_PARAMETER_H
