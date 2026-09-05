#include <cassert>
#include <filesystem>

#include "cad2sim/kernel/geometry_kernel.hpp"

int main() {
    const std::filesystem::path fixture =
        std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
        "tests" /
        "fixtures" /
        "step" /
        "screw.step";

    assert(std::filesystem::exists(fixture));

    cad2sim::kernel::GeometryKernel kernel;
    const auto counts = kernel.inspect_topology(fixture.string());

    assert(counts.solids > 0);
    assert(counts.shells > 0);
    assert(counts.faces > 0);
    assert(counts.wires > 0);
    assert(counts.edges > 0);
    assert(counts.vertices > 0);

    return 0;
}
