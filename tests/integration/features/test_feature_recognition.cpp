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

    assert(result.features.empty());

    return 0;
}
