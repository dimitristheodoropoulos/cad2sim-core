#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace cad2sim::kernel {

struct ImportResult {
    bool success;
    bool shape_valid;
    std::string error;
};

struct TopologyCounts {
    std::size_t solids;
    std::size_t shells;
    std::size_t faces;
    std::size_t wires;
    std::size_t edges;
    std::size_t vertices;
};

struct ValidationResult {
    bool valid;
    std::string diagnostic;
};

struct BoundingBox {
    double min_x;
    double min_y;
    double min_z;
    double max_x;
    double max_y;
    double max_z;
};

struct Point3 {
    double x;
    double y;
    double z;
};

struct Vector3 {
    double x;
    double y;
    double z;
};

struct ShapeProperties {
    BoundingBox bounding_box;
    double volume;
    Point3 centroid;
};

struct FaceProperties {
    std::size_t index;
    Vector3 normal;
    double area;
    Point3 centroid;
};

struct EdgeProperties {
    std::size_t index;
    double length;
};

struct GeometryProperties {
    BoundingBox bounding_box;
    double volume;
    Point3 centroid;
    std::vector<FaceProperties> faces;
    std::vector<EdgeProperties> edges;
};

enum class SurfaceType {
    Unknown,
    Plane,
    Cylinder
};

struct FaceDescriptor {
    // Zero-based index within the current face inspection result.
    std::size_t index;
    SurfaceType surface_type;
    double area;
    Point3 centroid;
};

class GeometryKernel {
public:
    ImportResult import_step(const std::string& path) const;
    TopologyCounts inspect_topology(const std::string& path) const;
    ValidationResult validate_step(const std::string& path) const;
    GeometryProperties inspect_geometry_properties(
        const std::string& path
    ) const;

    std::vector<FaceDescriptor> inspect_faces(
        const std::string& path
    ) const;
};

}  // namespace cad2sim::kernel
