#include <cassert>
#include <iostream>

#include "cad2sim/engineering/engineering_model.hpp"

using namespace cad2sim::engineering;

int main() {
    EngineeringModel model;

    model.regions = {
        {0, {RegionEntityType::Face, 0}},
        {1, {RegionEntityType::Face, 1}},
    };

    model.material_assignments = {
        {0, "steel"}
    };

    model.analysis_entities = {
        {"structural", {0, 1}}
    };

    model.boundary_condition_references = {
        {"fixed_support", {0}}
    };

    model.metadata = {
        {"source_format", "STEP"},
        {"preprocessing_stage", "engineering"}
    };

    EngineeringModelValidator validator;

    const auto valid_result =
        validator.validate(model);

    assert(valid_result.success);
    assert(valid_result.diagnostic.empty());

    auto invalid_material = model;
    invalid_material.material_assignments[0].region_index = 99;

    const auto invalid_material_result =
        validator.validate(invalid_material);

    assert(!invalid_material_result.success);
    assert(!invalid_material_result.diagnostic.empty());

    auto invalid_analysis = model;
    invalid_analysis.analysis_entities[0].region_indices = {0, 99};

    const auto invalid_analysis_result =
        validator.validate(invalid_analysis);

    assert(!invalid_analysis_result.success);
    assert(!invalid_analysis_result.diagnostic.empty());

    auto invalid_bc = model;
    invalid_bc.boundary_condition_references[0].region_indices = {99};

    const auto invalid_bc_result =
        validator.validate(invalid_bc);

    assert(!invalid_bc_result.success);
    assert(!invalid_bc_result.diagnostic.empty());

    auto invalid_metadata = model;
    invalid_metadata.metadata[0].key.clear();

    const auto invalid_metadata_result =
        validator.validate(invalid_metadata);

    assert(!invalid_metadata_result.success);
    assert(!invalid_metadata_result.diagnostic.empty());

    auto invalid_regions = model;
    invalid_regions.regions[1].index = 5;

    const auto invalid_regions_result =
        validator.validate(invalid_regions);

    assert(!invalid_regions_result.success);
    assert(!invalid_regions_result.diagnostic.empty());

    std::cout
        << "[PASS] engineering model validation unit contract\n";

    return 0;
}
