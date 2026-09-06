#pragma once

#include <cstddef>
#include <vector>

namespace cad2sim::mesh {

struct MeshNode {
    double x;
    double y;
    double z;
};

struct MeshTriangle {
    std::size_t node0;
    std::size_t node1;
    std::size_t node2;
};

struct FaceMesh {
    std::size_t face_index;
    std::vector<MeshNode> nodes;
    std::vector<MeshTriangle> triangles;
};

struct SurfaceMesh {
    std::vector<FaceMesh> faces;
};

}  // namespace cad2sim::mesh
