#include "cad2sim/features/feature_recognition.hpp"

#include <filesystem>

#include "cad2sim/kernel/geometry_kernel.hpp"

namespace cad2sim::features {

FeatureRecognitionResult FeatureRecognizer::recognize(
    const std::string& path
) const {
    FeatureRecognitionResult result;

    if (!std::filesystem::exists(path)) {
        return result;
    }

    cad2sim::kernel::GeometryKernel kernel;

    // Validate geometry first
    const auto validation = kernel.validate_step(path);
    if (!validation.valid) {
        return result;
    }

    // Get project-level face descriptors (no OCCT types exposed here)
    const auto face_descriptors = kernel.inspect_faces(path);

    for (const auto& desc : face_descriptors) {
        // Skip degenerate faces (area already computed by kernel)
        if (desc.area < 1e-12) {
            continue;
        }

        // Map CAD2Sim SurfaceType to FeatureType
        FeatureType type;
        switch (desc.surface_type) {
            case cad2sim::kernel::SurfaceType::Plane:
                type = FeatureType::PlanarFace;
                break;
            case cad2sim::kernel::SurfaceType::Cylinder:
                type = FeatureType::CylindricalFace;
                break;
            default:
                // Unknown surface types are ignored (not part of our feature set)
                continue;
        }

        result.features.push_back({
            type,
            FeatureStatus::Confirmed,
            desc.index,
            {}  // no additional metadata for now
        });
    }

    return result;
}

}  // namespace cad2sim::features
