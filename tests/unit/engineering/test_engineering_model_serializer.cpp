#include <cassert>
#include <iostream>
#include <string>

#include "cad2sim/engineering/engineering_model_serializer.hpp"

using namespace cad2sim::engineering;

namespace {

EngineeringModel make_model() {
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
        {"preprocessing_stage", "engineering"},
        {"source_format", "STEP"}
    };

    return model;
}

}  // namespace

int main() {
    EngineeringModelSerializer serializer;

    const EngineeringModel model = make_model();

    const auto first =
        serializer.serialize(model);

    assert(first.success);
    assert(!first.data.empty());
    assert(first.diagnostic.empty());

    const auto second =
        serializer.serialize(model);

    assert(second.success);
    assert(first.data == second.data);

    const auto deserialized =
        serializer.deserialize(first.data);

    assert(deserialized.success);
    assert(deserialized.diagnostic.empty());

    const auto round_trip =
        serializer.serialize(deserialized.model);

    assert(round_trip.success);
    assert(round_trip.data == first.data);

    const std::string malformed =
        "CAD2SIM_ENGINEERING_MODEL 1\n"
        "REGIONS 1\n"
        "REGION 0 FACE 0\n"
        "MATERIAL_ASSIGNMENTS 0\n"
        "ANALYSIS_ENTITIES 0\n"
        "BOUNDARY_CONDITION_REFERENCES 0\n"
        "METADATA 0\n";

    const auto malformed_result =
        serializer.deserialize(malformed);

    assert(!malformed_result.success);
    assert(!malformed_result.diagnostic.empty());

    const std::string unsupported_version =
        "CAD2SIM_ENGINEERING_MODEL 999\n";

    const auto version_result =
        serializer.deserialize(unsupported_version);

    assert(!version_result.success);
    assert(!version_result.diagnostic.empty());

    const std::string invalid_reference =
        "CAD2SIM_ENGINEERING_MODEL 1\n"
        "REGIONS 1\n"
        "REGION 0 FACE 0\n"
        "MATERIAL_ASSIGNMENTS 1\n"
        "MATERIAL 99 steel\n"
        "ANALYSIS_ENTITIES 0\n"
        "BOUNDARY_CONDITION_REFERENCES 0\n"
        "METADATA 0\n"
        "END\n";

    const auto invalid_reference_result =
        serializer.deserialize(invalid_reference);

    assert(!invalid_reference_result.success);
    assert(!invalid_reference_result.diagnostic.empty());

    const std::string missing_required_field =
        "CAD2SIM_ENGINEERING_MODEL 1\n"
        "REGIONS 1\n"
        "REGION 0 FACE 0\n"
        "MATERIAL_ASSIGNMENTS 0\n"
        "ANALYSIS_ENTITIES 0\n"
        "BOUNDARY_CONDITION_REFERENCES 0\n"
        "METADATA 1\n"
        "END\n";

    const auto missing_field_result =
        serializer.deserialize(missing_required_field);

    assert(!missing_field_result.success);
    assert(!missing_field_result.diagnostic.empty());

    std::cout
        << "[PASS] engineering model serialization unit contract\n";

    return 0;
}
