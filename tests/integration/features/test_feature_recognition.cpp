#include <cassert>
#include <filesystem>

#include "cad2sim/features/feature_recognition.hpp"

namespace {

std::filesystem::path fixture(const char* name) {
    return std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
           "tests" /
           "fixtures" /
           "step" /
           name;
}

bool has_feature(
    const cad2sim::features::FeatureRecognitionResult& result,
    cad2sim::features::FeatureType type,
    cad2sim::features::FeatureStatus status
) {
    for (const auto& feature : result.features) {
        if (feature.type == type && feature.status == status) {
            return true;
        }
    }

    return false;
}

bool has_type(
    const cad2sim::features::FeatureRecognitionResult& result,
    cad2sim::features::FeatureType type
) {
    for (const auto& feature : result.features) {
        if (feature.type == type) {
            return true;
        }
    }

    return false;
}

void assert_no_type(
    const cad2sim::features::FeatureRecognitionResult& result,
    cad2sim::features::FeatureType type
) {
    assert(!has_type(result, type));
}

} // namespace

int main() {
    using cad2sim::features::FeatureRecognizer;
    using cad2sim::features::FeatureStatus;
    using cad2sim::features::FeatureType;

    FeatureRecognizer recognizer;

    // ------------------------------------------------------------
    // Existing surface-level recognition must remain intact.
    // ------------------------------------------------------------

    const auto screw = recognizer.recognize(
        fixture("screw.step").string()
    );

    assert(has_feature(
        screw,
        FeatureType::PlanarFace,
        FeatureStatus::Confirmed
    ));

    assert(has_feature(
        screw,
        FeatureType::CylindricalFace,
        FeatureStatus::Confirmed
    ));

    // ------------------------------------------------------------
    // HOLE-001
    // Through cylindrical cavity: Hole / Confirmed.
    // A Hole must not be confused with Pocket, Fillet or Chamfer.
    // ------------------------------------------------------------

    const auto hole = recognizer.recognize(
        fixture("hole-001.step").string()
    );

    assert(has_feature(
        hole,
        FeatureType::Hole,
        FeatureStatus::Confirmed
    ));

    assert_no_type(hole, FeatureType::Pocket);
    assert_no_type(hole, FeatureType::Fillet);
    assert_no_type(hole, FeatureType::Chamfer);

    // ------------------------------------------------------------
    // POCKET-001
    // Blind cylindrical cavity: Pocket / Confirmed.
    // Critical differential: Pocket must not be classified as Hole.
    // ------------------------------------------------------------

    const auto pocket = recognizer.recognize(
        fixture("pocket-001.step").string()
    );

    assert(has_feature(
        pocket,
        FeatureType::Pocket,
        FeatureStatus::Confirmed
    ));

    assert_no_type(pocket, FeatureType::Hole);
    assert_no_type(pocket, FeatureType::Fillet);
    assert_no_type(pocket, FeatureType::Chamfer);

    // ------------------------------------------------------------
    // FILLET-001
    // External rounded transition: Fillet / Confirmed.
    // A cylindrical surface alone must not imply Hole.
    // ------------------------------------------------------------

    const auto fillet = recognizer.recognize(
        fixture("fillet-001.step").string()
    );

    assert(has_feature(
        fillet,
        FeatureType::Fillet,
        FeatureStatus::Confirmed
    ));

    assert_no_type(fillet, FeatureType::Hole);
    assert_no_type(fillet, FeatureType::Pocket);
    assert_no_type(fillet, FeatureType::Chamfer);

    // ------------------------------------------------------------
    // CHAMFER-001
    // External planar bevel: Chamfer / Confirmed.
    // A planar surface alone must not imply Chamfer.
    // ------------------------------------------------------------

    const auto chamfer = recognizer.recognize(
        fixture("chamfer-001.step").string()
    );

    assert(has_feature(
        chamfer,
        FeatureType::Chamfer,
        FeatureStatus::Confirmed
    ));

    assert_no_type(chamfer, FeatureType::Hole);
    assert_no_type(chamfer, FeatureType::Pocket);
    assert_no_type(chamfer, FeatureType::Fillet);

    // ------------------------------------------------------------
    // NEG-001-EXTERNAL-BOSS
    // External cylindrical boss: cylindrical surface is present,
    // but it must not be classified as Hole or Pocket.
    // ------------------------------------------------------------

    const auto external_boss = recognizer.recognize(
        fixture("neg-001-external-boss.step").string()
    );

    assert(has_feature(
        external_boss,
        FeatureType::CylindricalFace,
        FeatureStatus::Confirmed
    ));

    assert_no_type(external_boss, FeatureType::Hole);
    assert_no_type(external_boss, FeatureType::Pocket);

    // ------------------------------------------------------------
    // NEG-002-SHARP-EDGE
    // Plain sharp-edged block: no rounded transition exists,
    // therefore it must not be classified as Fillet.
    // ------------------------------------------------------------

    const auto sharp_edge = recognizer.recognize(
        fixture("neg-002-sharp-edge.step").string()
    );

    assert_no_type(sharp_edge, FeatureType::Fillet);

    // ------------------------------------------------------------
    // NEG-003-PLAIN-PLANAR
    // Plain block: planar faces alone must not imply Chamfer.
    // ------------------------------------------------------------

    const auto plain_planar = recognizer.recognize(
        fixture("neg-003-plain-planar.step").string()
    );

    assert_no_type(plain_planar, FeatureType::Chamfer);

    // ------------------------------------------------------------
    // NEG-005-UNSUPPORTED-SURFACE
    // Spherical surface is outside the currently supported
    // Plane/Cylinder surface set and must be reported explicitly.
    // ------------------------------------------------------------

    const auto unsupported_surface = recognizer.recognize(
        fixture("neg-005-unsupported-surface.step").string()
    );

    assert(has_feature(
        unsupported_surface,
        FeatureType::Unknown,
        FeatureStatus::Unsupported
    ));

    bool has_unsupported_diagnostic = false;

    for (const auto& feature : unsupported_surface.features) {
        if (feature.type == FeatureType::Unknown &&
            feature.status == FeatureStatus::Unsupported &&
            !feature.diagnostic.empty()) {
            has_unsupported_diagnostic = true;
            break;
        }
    }

    assert(has_unsupported_diagnostic);
    return 0;
}
