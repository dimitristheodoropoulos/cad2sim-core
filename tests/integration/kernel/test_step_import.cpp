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

    const auto result = kernel.import_step(fixture.string());

    assert(result.success);
    assert(result.shape_valid);

    const std::filesystem::path missing_fixture =
        std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
        "tests" /
        "fixtures" /
        "step" /
        "missing.step";

    assert(!std::filesystem::exists(missing_fixture));

    const auto missing_result = kernel.import_step(missing_fixture.string());

    assert(!missing_result.success);
    assert(!missing_result.shape_valid);
    assert(!missing_result.error.empty());
    assert(
        missing_result.error ==
        "STEP file does not exist: " + missing_fixture.string()
    );

    return 0;
}
