#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <limits>

#include "cad2sim/kernel/geometry_kernel.hpp"
#include "cad2sim/mesh/surface_mesher.hpp"

namespace {

std::filesystem::path fixture(
    const char* name
) {
    return std::filesystem::path(
        CAD2SIM_TEST_SOURCE_DIR
    ) / "tests" / "fixtures" / "step" / name;
}

void test_valid_mesh() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::mesh::SurfaceMesher mesher;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);
    assert(loaded.shape.valid());
    assert(loaded.diagnostic.empty());

    const auto result =
        mesher.mesh(loaded.shape);

    assert(result.success);
    assert(!result.mesh.faces.empty());
    assert(result.diagnostic.empty());

    for (const auto& face : result.mesh.faces) {
        assert(!face.nodes.empty());
        assert(!face.triangles.empty());

        for (const auto& triangle : face.triangles) {
            assert(triangle.node0 < face.nodes.size());
            assert(triangle.node1 < face.nodes.size());
            assert(triangle.node2 < face.nodes.size());
        }

        for (const auto& node : face.nodes) {
            assert(std::isfinite(node.x));
            assert(std::isfinite(node.y));
            assert(std::isfinite(node.z));
        }
    }
}

void test_deterministic_structure() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::mesh::SurfaceMesher mesher;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);

    cad2sim::mesh::SurfaceMeshParameters parameters;

    const auto first =
        mesher.mesh(
            loaded.shape,
            parameters
        );

    const auto second =
        mesher.mesh(
            loaded.shape,
            parameters
        );

    assert(first.success);
    assert(second.success);

    assert(
        first.mesh.faces.size() ==
        second.mesh.faces.size()
    );

    for (std::size_t i = 0;
         i < first.mesh.faces.size();
         ++i) {

        const auto& a = first.mesh.faces[i];
        const auto& b = second.mesh.faces[i];

        assert(a.face_index == b.face_index);
        assert(a.nodes.size() == b.nodes.size());
        assert(a.triangles.size() == b.triangles.size());

        for (std::size_t j = 0;
             j < a.nodes.size();
             ++j) {
            assert(a.nodes[j].x == b.nodes[j].x);
            assert(a.nodes[j].y == b.nodes[j].y);
            assert(a.nodes[j].z == b.nodes[j].z);
        }

        for (std::size_t j = 0;
             j < a.triangles.size();
             ++j) {
            assert(
                a.triangles[j].node0 ==
                b.triangles[j].node0
            );
            assert(
                a.triangles[j].node1 ==
                b.triangles[j].node1
            );
            assert(
                a.triangles[j].node2 ==
                b.triangles[j].node2
            );
        }
    }
}

void test_invalid_parameters() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::mesh::SurfaceMesher mesher;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);

    {
        cad2sim::mesh::SurfaceMeshParameters parameters;
        parameters.linear_deflection = 0.0;

        const auto result =
            mesher.mesh(
                loaded.shape,
                parameters
            );

        assert(!result.success);
        assert(!result.diagnostic.empty());
    }

    {
        cad2sim::mesh::SurfaceMeshParameters parameters;
        parameters.angular_deflection = -1.0;

        const auto result =
            mesher.mesh(
                loaded.shape,
                parameters
            );

        assert(!result.success);
        assert(!result.diagnostic.empty());
    }

    {
        cad2sim::mesh::SurfaceMeshParameters parameters;
        parameters.linear_deflection =
            std::numeric_limits<double>::quiet_NaN();

        const auto result =
            mesher.mesh(
                loaded.shape,
                parameters
            );

        assert(!result.success);
        assert(!result.diagnostic.empty());
    }
}

void test_invalid_shape() {
    cad2sim::mesh::SurfaceMesher mesher;

    cad2sim::kernel::ValidatedShape invalid_shape;

    const auto result =
        mesher.mesh(invalid_shape);

    assert(!result.success);
    assert(!result.diagnostic.empty());
}

void test_missing_step() {
    cad2sim::kernel::GeometryKernel kernel;

    const auto result =
        kernel.load_validated_shape(
            "/tmp/cad2sim-req011-does-not-exist.step"
        );

    assert(!result.success);
    assert(!result.shape.valid());
    assert(!result.diagnostic.empty());
}

}  // namespace

int main() {
    test_valid_mesh();
    test_deterministic_structure();
    test_invalid_parameters();
    test_invalid_shape();
    test_missing_step();

    std::cout
        << "[PASS] surface mesh integration\n";

    return 0;
}
