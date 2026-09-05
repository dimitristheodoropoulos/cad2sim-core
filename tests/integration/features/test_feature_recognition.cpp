#include <cassert>
#include <filesystem>

#include "cad2sim/features/feature_recognition.hpp"

int main() {
    const std::filesystem::path fixture =
        std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
        "tests" /
        "fixtures" /
        "step" /
        "screw.step";

    assert(std::filesystem::exists(fixture));

    cad2sim::features::FeatureRecognizer recognizer;
    const auto result = recognizer.recognize(fixture.string());

    std::size_t planar_faces = 0;
    std::size_t cylindrical_faces = 0;

    for (const auto& feature : result.features) {
        if (feature.status !=
            cad2sim::features::FeatureStatus::Confirmed) {
            continue;
        }

        if (feature.type ==
            cad2sim::features::FeatureType::PlanarFace) {
            ++planar_faces;
        }

        if (feature.type ==
            cad2sim::features::FeatureType::CylindricalFace) {
            ++cylindrical_faces;
        }
    }

    assert(planar_faces > 0);
    assert(cylindrical_faces > 0);

    return 0;
}
