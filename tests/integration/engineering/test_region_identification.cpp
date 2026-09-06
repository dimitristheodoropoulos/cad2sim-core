#include <cassert>
#include <filesystem>
#include <iostream>

#include "cad2sim/engineering/region_identification.hpp"
#include "cad2sim/kernel/geometry_kernel.hpp"

namespace {

std::filesystem::path fixture(
    const char* name
) {
    return std::filesystem::path(
        CAD2SIM_TEST_SOURCE_DIR
    ) / "tests" / "fixtures" / "step" / name;
}

void test_region_identification() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::engineering::RegionIdentifier identifier;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);
    assert(loaded.shape.valid());
    assert(loaded.diagnostic.empty());

    const auto result =
        identifier.identify(loaded.shape);

    assert(result.success);
    assert(!result.regions.empty());
    assert(result.diagnostic.empty());

    const auto topology =
        kernel.inspect_face_topology(
            fixture("screw.step").string()
        );

    assert(!topology.faces.empty());
    assert(result.regions.size() == topology.faces.size());

    for (std::size_t i = 0;
         i < result.regions.size();
         ++i) {
        const auto& region = result.regions[i];

        assert(region.index == i);
        assert(
            region.source.entity_type ==
            cad2sim::engineering::RegionEntityType::Face
        );
        assert(region.source.entity_index < topology.faces.size());
    }
}

void test_deterministic_identification() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::engineering::RegionIdentifier identifier;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);

    const auto first =
        identifier.identify(loaded.shape);

    const auto second =
        identifier.identify(loaded.shape);

    assert(first.success);
    assert(second.success);

    assert(
        first.regions.size() ==
        second.regions.size()
    );

    for (std::size_t i = 0;
         i < first.regions.size();
         ++i) {
        assert(
            first.regions[i].index ==
            second.regions[i].index
        );
        assert(
            first.regions[i].source.entity_type ==
            second.regions[i].source.entity_type
        );
        assert(
            first.regions[i].source.entity_index ==
            second.regions[i].source.entity_index
        );
    }
}

void test_invalid_shape() {
    cad2sim::engineering::RegionIdentifier identifier;

    cad2sim::kernel::ValidatedShape invalid_shape;

    const auto result =
        identifier.identify(invalid_shape);

    assert(!result.success);
    assert(result.regions.empty());
    assert(!result.diagnostic.empty());
}

}  // namespace

int main() {
    test_region_identification();
    test_deterministic_identification();
    test_invalid_shape();

    std::cout
        << "[PASS] engineering region identification integration contract\n";

    return 0;
}
