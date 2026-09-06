#include <cassert>
#include <iostream>

#include "cad2sim/kernel/geometry_kernel.hpp"
#include "cad2sim/mesh/mesh_quality.hpp"
#include "cad2sim/mesh/surface_mesher.hpp"

#ifndef CAD2SIM_TEST_SOURCE_DIR
#error "CAD2SIM_TEST_SOURCE_DIR must be defined"
#endif

int main()
{
    const std::string step_path =
        std::string(CAD2SIM_TEST_SOURCE_DIR) +
        "/tests/fixtures/step/screw.step";

    cad2sim::kernel::GeometryKernel kernel;

    const auto loaded = kernel.load_validated_shape(step_path);

    assert(loaded.success);
    assert(loaded.shape.valid());

    cad2sim::mesh::SurfaceMesher mesher;

    const auto meshed = mesher.mesh(loaded.shape);

    assert(meshed.success);
    assert(!meshed.mesh.faces.empty());

    const auto quality =
        cad2sim::mesh::MeshQualityEvaluator{}.evaluate(meshed.mesh);

    assert(quality.success);
    assert(quality.element_count > 0);
    assert(
        quality.elements.size() ==
        quality.element_count);

    std::cout << "[PASS] mesh quality integration contract\n";
    return 0;
}
