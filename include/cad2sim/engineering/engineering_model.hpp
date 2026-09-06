#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "cad2sim/engineering/region_identification.hpp"

namespace cad2sim::engineering {

struct MaterialAssignment {
    std::size_t region_index;
    std::string material_id;
};

struct AnalysisEntity {
    std::string id;
    std::vector<std::size_t> region_indices;
};

struct BoundaryConditionReference {
    std::string id;
    std::vector<std::size_t> region_indices;
};

struct PreprocessingMetadata {
    std::string key;
    std::string value;
};

struct EngineeringModel {
    std::vector<EngineeringRegion> regions;
    std::vector<MaterialAssignment> material_assignments;
    std::vector<AnalysisEntity> analysis_entities;
    std::vector<BoundaryConditionReference>
        boundary_condition_references;
    std::vector<PreprocessingMetadata> metadata;
};

struct EngineeringModelValidationResult {
    bool success;
    std::string diagnostic;
};

class EngineeringModelValidator {
public:
    EngineeringModelValidationResult validate(
        const EngineeringModel& model
    ) const;
};

}  // namespace cad2sim::engineering
