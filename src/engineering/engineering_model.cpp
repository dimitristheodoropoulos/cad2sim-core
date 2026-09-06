#include "cad2sim/engineering/engineering_model.hpp"

#include <sstream>

namespace cad2sim::engineering {

namespace {

bool contains_region(
    const EngineeringModel& model,
    std::size_t region_index
) {
    for (const auto& region : model.regions) {
        if (region.index == region_index) {
            return true;
        }
    }

    return false;
}

bool valid_region_layout(
    const EngineeringModel& model
) {
    for (std::size_t index = 0;
         index < model.regions.size();
         ++index) {

        if (model.regions[index].index != index) {
            return false;
        }
    }

    return true;
}

}  // namespace

EngineeringModelValidationResult
EngineeringModelValidator::validate(
    const EngineeringModel& model
) const {
    if (!valid_region_layout(model)) {
        return {
            false,
            "Engineering model validation failed: region indices must be contiguous"
        };
    }

    for (const auto& assignment : model.material_assignments) {
        if (assignment.material_id.empty()) {
            return {
                false,
                "Engineering model validation failed: material identifier is empty"
            };
        }

        if (!contains_region(model, assignment.region_index)) {
            std::ostringstream diagnostic;
            diagnostic
                << "Engineering model validation failed: material assignment "
                << "references unknown region "
                << assignment.region_index;

            return {
                false,
                diagnostic.str()
            };
        }
    }

    for (const auto& entity : model.analysis_entities) {
        if (entity.id.empty()) {
            return {
                false,
                "Engineering model validation failed: analysis entity identifier is empty"
            };
        }

        for (const auto region_index : entity.region_indices) {
            if (!contains_region(model, region_index)) {
                std::ostringstream diagnostic;
                diagnostic
                    << "Engineering model validation failed: analysis entity "
                    << entity.id
                    << " references unknown region "
                    << region_index;

                return {
                    false,
                    diagnostic.str()
                };
            }
        }
    }

    for (const auto& reference :
         model.boundary_condition_references) {

        if (reference.id.empty()) {
            return {
                false,
                "Engineering model validation failed: boundary-condition reference identifier is empty"
            };
        }

        for (const auto region_index : reference.region_indices) {
            if (!contains_region(model, region_index)) {
                std::ostringstream diagnostic;
                diagnostic
                    << "Engineering model validation failed: boundary-condition reference "
                    << reference.id
                    << " references unknown region "
                    << region_index;

                return {
                    false,
                    diagnostic.str()
                };
            }
        }
    }

    for (const auto& metadata : model.metadata) {
        if (metadata.key.empty()) {
            return {
                false,
                "Engineering model validation failed: metadata key is empty"
            };
        }
    }

    return {
        true,
        {}
    };
}

}  // namespace cad2sim::engineering
