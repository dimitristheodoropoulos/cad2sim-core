#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "cad2sim/mesh/surface_mesh.hpp"

namespace cad2sim::mesh {

struct ElementQuality {
    std::size_t face_index;
    std::size_t element_index;

    double area;
    double min_edge_length;
    double max_edge_length;
    double aspect_ratio;

    double min_angle_degrees;
    double max_angle_degrees;

    bool degenerate;
};

struct MeshQualityResult {
    bool success;
    std::size_t element_count;
    std::vector<ElementQuality> elements;
    std::string diagnostic;
};

class MeshQualityEvaluator {
public:
    MeshQualityResult evaluate(
        const SurfaceMesh& mesh
    ) const;
};

}  // namespace cad2sim::mesh
