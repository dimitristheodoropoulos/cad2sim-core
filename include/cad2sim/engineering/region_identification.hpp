#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "cad2sim/kernel/validated_shape.hpp"

namespace cad2sim::engineering {

enum class RegionEntityType {
    Face
};

struct RegionReference {
    RegionEntityType entity_type;
    std::size_t entity_index;
};

struct EngineeringRegion {
    std::size_t index;
    RegionReference source;
};

struct RegionIdentificationResult {
    bool success;
    std::vector<EngineeringRegion> regions;
    std::string diagnostic;
};

class RegionIdentifier {
public:
    RegionIdentificationResult identify(
        const kernel::ValidatedShape& shape
    ) const;
};

}  // namespace cad2sim::engineering
