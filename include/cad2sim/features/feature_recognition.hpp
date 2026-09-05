#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cad2sim::features {

enum class FeatureType {
    PlanarFace,
    CylindricalFace,
    Hole,
    Pocket,
    Fillet,
    Chamfer
};

enum class FeatureStatus {
    Confirmed,
    Ambiguous,
    Unsupported
};

struct Feature {
    FeatureType type;
    FeatureStatus status;
    std::size_t index;
    std::string diagnostic;
};

struct FeatureRecognitionResult {
    std::vector<Feature> features;
};

class FeatureRecognizer {
public:
    FeatureRecognitionResult recognize(const std::string& path) const;
};

}  // namespace cad2sim::features
