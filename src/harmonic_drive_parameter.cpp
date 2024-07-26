#include "harmonic_drive_parameter.h"

// Implementation of the constructor using HarmonicDriveParameterType
HarmonicDriveParameters::HarmonicDriveParameters(double value, HarmonicDriveParameterType t)
    : offset(0.0), slope(0.0), constant(0.0), type(Type::Undefined) {
    if (t == HarmonicDriveParameterType::Offset) { 
        offset = value; type = Type::Offset; 
    } else if (t == HarmonicDriveParameterType::Slope) { 
        slope = value; type = Type::Slope; 
    } else if (t == HarmonicDriveParameterType::Constant) { 
        constant = value; type = Type::Constant; 
    } else {
        throw std::invalid_argument("Invalid type for this constructor");
    }
}

// to_string function for HarmonicDriveParameters
std::string to_string(const HarmonicDriveParameters &params) {
    std::ostringstream oss;
    if (params.isConstant()) {
        oss << "Constant: " << params.getConstant();
    } else if (params.isOffsetAndSlope()) {
        oss << "Offset: " << params.getOffset() << ", Slope: " << params.getSlope();
    } else if (params.isOffset()) {
        oss << "Offset: " << params.getOffset();
    } else if (params.isSlope()) {
        oss << "Slope: " << params.getSlope();
    } else {
        oss << "Undefined";
    }
    return oss.str();
}

//  inline functions

HarmonicDriveParameters::HarmonicDriveParameters() : type(Type::Undefined), offset(0.0), slope(0.0), constant(0.0) {}

HarmonicDriveParameters::HarmonicDriveParameters(double offset, double slope) : type(Type::OffsetAndSlope), offset(offset), slope(slope), constant(0.0) {}

double HarmonicDriveParameters::getConstant() const {
    if (type != Type::Constant) throw std::logic_error("Not a constant parameter");
    return constant;
}

double HarmonicDriveParameters::getOffset() const {
    if (type == Type::Offset || type == Type::OffsetAndSlope) return offset;
    throw std::logic_error("Not an offset parameter");
}

double HarmonicDriveParameters::getSlope() const {
    if (type == Type::Slope || type == Type::OffsetAndSlope) return slope;
    throw std::logic_error("Not a slope parameter");
}

double HarmonicDriveParameters::get(HarmonicDriveParameterType type) const {
    if(!isType(type)) throw std::runtime_error("Tried to get wrong type");
    switch (type) {
        case HarmonicDriveParameterType::Constant:
            return constant;
        case HarmonicDriveParameterType::Slope:
            return slope;
        case HarmonicDriveParameterType::Offset:
            return offset;
        default:
            throw std::runtime_error("Tried to get wrong type");
    }
}

bool HarmonicDriveParameters::isConstant() const { return type == Type::Constant; }
bool HarmonicDriveParameters::isOffset() const { return type == Type::Offset || type == Type::OffsetAndSlope; }
bool HarmonicDriveParameters::isSlope() const { return type == Type::Slope || type == Type::OffsetAndSlope; }
bool HarmonicDriveParameters::isOffsetAndSlope() const { return type == Type::OffsetAndSlope; }
bool HarmonicDriveParameters::isType(HarmonicDriveParameterType t) const { 
    switch (t) {
        case HarmonicDriveParameterType::Constant:
            return isConstant();
        case HarmonicDriveParameterType::Slope:
            return isSlope();
        case HarmonicDriveParameterType::Offset:
            return isOffset();
        default:
            return false;
    }
}


void HarmonicDriveParameters::setValue(double value, HarmonicDriveParameterType t) {
    if(!isType(t)) throw std::runtime_error("Tried to set wrong type");
    switch (t) {
        case HarmonicDriveParameterType::Constant:
            constant = value;
            break;
        case HarmonicDriveParameterType::Slope:
            slope = value;
            break;
        case HarmonicDriveParameterType::Offset:
            offset = value;
            break;
    }
}

bool HarmonicDriveParameters::operator==(const HarmonicDriveParameters& other) const {
    if (type != other.type) return false;
    switch (type) {
        case Type::Offset:
            return offset == other.offset;
        case Type::Slope:
            return slope == other.slope;
        case Type::Constant:
            return constant == other.constant;
        case Type::OffsetAndSlope:
            return offset == other.offset && slope == other.slope;
        default:
            return false;
    }
}
