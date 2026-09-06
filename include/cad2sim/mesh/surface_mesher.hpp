#pragma once

#include <string>

#include "cad2sim/kernel/validated_shape.hpp"
#include "cad2sim/mesh/surface_mesh.hpp"

namespace cad2sim::mesh {

struct SurfaceMeshParameters {
    double linear_deflection = 0.1;
    double angular_deflection = 0.5;
};

struct SurfaceMeshResult {
    bool success;
    SurfaceMesh mesh;
    std::string diagnostic;
};

class SurfaceMesher {
public:
    SurfaceMeshResult mesh(
        const kernel::ValidatedShape& shape,
        const SurfaceMeshParameters& parameters = {}
    ) const;
};

}  // namespace cad2sim::mesh
