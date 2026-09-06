#pragma once

#include <string>

#include "cad2sim/engineering/engineering_model.hpp"

namespace cad2sim::engineering {

struct EngineeringModelSerializationResult {
    bool success;
    std::string data;
    std::string diagnostic;
};

struct EngineeringModelDeserializationResult {
    bool success;
    EngineeringModel model;
    std::string diagnostic;
};

class EngineeringModelSerializer {
public:
    EngineeringModelSerializationResult serialize(
        const EngineeringModel& model
    ) const;

    EngineeringModelDeserializationResult deserialize(
        const std::string& data
    ) const;
};

}  // namespace cad2sim::engineering
