#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "cad2sim/engineering/engineering_model.hpp"
#include "cad2sim/engineering/engineering_model_serializer.hpp"
#include "cad2sim/engineering/region_identification.hpp"
#include "cad2sim/kernel/geometry_kernel.hpp"
#include "cad2sim/mesh/mesh_quality.hpp"
#include "cad2sim/mesh/surface_mesher.hpp"

namespace {

std::filesystem::path fixture(const char* name) {
    return std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
           "tests" / "fixtures" / "step" / name;
}

struct PipelineSnapshot {
    bool validation_success;
    std::string validation_diagnostic;

    bool mesh_success;
    std::string mesh_diagnostic;
    cad2sim::mesh::SurfaceMesh mesh;

    bool quality_success;
    std::string quality_diagnostic;
    std::size_t quality_element_count;
    std::vector<cad2sim::mesh::ElementQuality> quality_elements;

    bool regions_success;
    std::string regions_diagnostic;
    std::vector<cad2sim::engineering::EngineeringRegion> regions;

    bool model_validation_success;
    std::string model_validation_diagnostic;

    std::string serialized_model;
};

cad2sim::engineering::EngineeringModel make_engineering_model(
    const std::vector<cad2sim::engineering::EngineeringRegion>& regions
) {
    cad2sim::engineering::EngineeringModel model;
    model.regions = regions;

    if (!regions.empty()) {
        model.material_assignments.push_back(
            {0, "steel"}
        );

        std::vector<std::size_t> all_regions;
        all_regions.reserve(regions.size());

        for (const auto& region : regions) {
            all_regions.push_back(region.index);
        }

        model.analysis_entities.push_back(
            {"structural", all_regions}
        );

        model.boundary_condition_references.push_back(
            {"fixed_support", {regions.front().index}}
        );
    }

    model.metadata = {
        {"preprocessing_stage", "determinism"},
        {"source_format", "STEP"}
    };

    return model;
}

PipelineSnapshot run_pipeline() {
    using namespace cad2sim;

    PipelineSnapshot snapshot{};

    const auto step_path = fixture("screw.step").string();

    kernel::GeometryKernel kernel;
    mesh::SurfaceMesher mesher;
    mesh::MeshQualityEvaluator quality_evaluator;
    engineering::RegionIdentifier region_identifier;
    engineering::EngineeringModelValidator model_validator;
    engineering::EngineeringModelSerializer serializer;

    /*
     * Explicit processing parameters are part of the deterministic
     * pipeline input. Do not rely on backend defaults.
     */
    mesh::SurfaceMeshParameters parameters;
    parameters.linear_deflection = 0.1;
    parameters.angular_deflection = 0.5;

    const auto loaded =
        kernel.load_validated_shape(step_path);

    snapshot.validation_success = loaded.success;
    snapshot.validation_diagnostic = loaded.diagnostic;

    assert(snapshot.validation_success);
    assert(loaded.shape.valid());
    assert(snapshot.validation_diagnostic.empty());

    const auto mesh_result =
        mesher.mesh(
            loaded.shape,
            parameters
        );

    snapshot.mesh_success = mesh_result.success;
    snapshot.mesh_diagnostic = mesh_result.diagnostic;
    snapshot.mesh = mesh_result.mesh;

    assert(snapshot.mesh_success);
    assert(!snapshot.mesh.faces.empty());
    assert(snapshot.mesh_diagnostic.empty());

    const auto quality_result =
        quality_evaluator.evaluate(snapshot.mesh);

    snapshot.quality_success = quality_result.success;
    snapshot.quality_diagnostic = quality_result.diagnostic;
    snapshot.quality_element_count =
        quality_result.element_count;
    snapshot.quality_elements =
        quality_result.elements;

    assert(snapshot.quality_success);
    assert(
        snapshot.quality_element_count ==
        snapshot.quality_elements.size()
    );
    assert(snapshot.quality_diagnostic.empty());

    const auto region_result =
        region_identifier.identify(loaded.shape);

    snapshot.regions_success = region_result.success;
    snapshot.regions_diagnostic = region_result.diagnostic;
    snapshot.regions = region_result.regions;

    assert(snapshot.regions_success);
    assert(!snapshot.regions.empty());
    assert(snapshot.regions_diagnostic.empty());

    const auto model =
        make_engineering_model(snapshot.regions);

    const auto model_validation =
        model_validator.validate(model);

    snapshot.model_validation_success =
        model_validation.success;
    snapshot.model_validation_diagnostic =
        model_validation.diagnostic;

    assert(snapshot.model_validation_success);
    assert(snapshot.model_validation_diagnostic.empty());

    const auto serialized =
        serializer.serialize(model);

    assert(serialized.success);
    assert(!serialized.data.empty());
    assert(serialized.diagnostic.empty());

    snapshot.serialized_model = serialized.data;

    return snapshot;
}

void compare_mesh(
    const cad2sim::mesh::SurfaceMesh& first,
    const cad2sim::mesh::SurfaceMesh& second
) {
    assert(first.faces.size() == second.faces.size());

    for (std::size_t i = 0; i < first.faces.size(); ++i) {
        const auto& a = first.faces[i];
        const auto& b = second.faces[i];

        assert(a.face_index == b.face_index);
        assert(a.nodes.size() == b.nodes.size());
        assert(a.triangles.size() == b.triangles.size());

        for (std::size_t j = 0; j < a.nodes.size(); ++j) {
            assert(a.nodes[j].x == b.nodes[j].x);
            assert(a.nodes[j].y == b.nodes[j].y);
            assert(a.nodes[j].z == b.nodes[j].z);
        }

        for (std::size_t j = 0; j < a.triangles.size(); ++j) {
            assert(a.triangles[j].node0 == b.triangles[j].node0);
            assert(a.triangles[j].node1 == b.triangles[j].node1);
            assert(a.triangles[j].node2 == b.triangles[j].node2);
        }
    }
}

void compare_quality(
    const std::vector<cad2sim::mesh::ElementQuality>& first,
    const std::vector<cad2sim::mesh::ElementQuality>& second
) {
    assert(first.size() == second.size());

    for (std::size_t i = 0; i < first.size(); ++i) {
        const auto& a = first[i];
        const auto& b = second[i];

        assert(a.face_index == b.face_index);
        assert(a.element_index == b.element_index);

        assert(a.area == b.area);
        assert(a.min_edge_length == b.min_edge_length);
        assert(a.max_edge_length == b.max_edge_length);
        assert(a.aspect_ratio == b.aspect_ratio);
        assert(a.min_angle_degrees == b.min_angle_degrees);
        assert(a.max_angle_degrees == b.max_angle_degrees);
        assert(a.degenerate == b.degenerate);

        assert(std::isfinite(a.area));
        assert(std::isfinite(a.aspect_ratio));
        assert(std::isfinite(a.min_angle_degrees));
        assert(std::isfinite(a.max_angle_degrees));
    }
}

void compare_regions(
    const std::vector<cad2sim::engineering::EngineeringRegion>& first,
    const std::vector<cad2sim::engineering::EngineeringRegion>& second
) {
    assert(first.size() == second.size());

    for (std::size_t i = 0; i < first.size(); ++i) {
        assert(first[i].index == second[i].index);
        assert(
            first[i].source.entity_type ==
            second[i].source.entity_type
        );
        assert(
            first[i].source.entity_index ==
            second[i].source.entity_index
        );
    }
}

void test_pipeline_determinism() {
    const auto first = run_pipeline();
    const auto second = run_pipeline();

    assert(
        first.validation_success ==
        second.validation_success
    );
    assert(
        first.validation_diagnostic ==
        second.validation_diagnostic
    );

    assert(
        first.mesh_success ==
        second.mesh_success
    );
    assert(
        first.mesh_diagnostic ==
        second.mesh_diagnostic
    );

    compare_mesh(first.mesh, second.mesh);

    assert(
        first.quality_success ==
        second.quality_success
    );
    assert(
        first.quality_diagnostic ==
        second.quality_diagnostic
    );
    assert(
        first.quality_element_count ==
        second.quality_element_count
    );

    compare_quality(
        first.quality_elements,
        second.quality_elements
    );

    assert(
        first.regions_success ==
        second.regions_success
    );
    assert(
        first.regions_diagnostic ==
        second.regions_diagnostic
    );

    compare_regions(
        first.regions,
        second.regions
    );

    assert(
        first.model_validation_success ==
        second.model_validation_success
    );
    assert(
        first.model_validation_diagnostic ==
        second.model_validation_diagnostic
    );

    /*
     * This is the application-level deterministic artifact required
     * by REQ-015: identical input + identical parameters must produce
     * identical serialized engineering output.
     */
    assert(
        first.serialized_model ==
        second.serialized_model
    );
}

}  // namespace

int main() {
    test_pipeline_determinism();

    std::cout
        << "[PASS] preprocessing determinism regression\n";

    return 0;
}
