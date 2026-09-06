#include "cad2sim/mesh/mesh_quality.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace cad2sim::mesh {
namespace {

constexpr double kAreaTolerance = 1e-12;

struct Vector3 {
    double x;
    double y;
    double z;
};

Vector3 subtract(const MeshNode& a, const MeshNode& b)
{
    return {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    };
}

double dot(const Vector3& a, const Vector3& b)
{
    return
        a.x * b.x +
        a.y * b.y +
        a.z * b.z;
}

Vector3 cross(const Vector3& a, const Vector3& b)
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double length(const Vector3& value)
{
    return std::sqrt(dot(value, value));
}

bool finite_node(const MeshNode& node)
{
    return
        std::isfinite(node.x) &&
        std::isfinite(node.y) &&
        std::isfinite(node.z);
}

double clamp_cosine(double value)
{
    return std::clamp(value, -1.0, 1.0);
}

double angle_degrees(
    const Vector3& first,
    const Vector3& second)
{
    const double first_length = length(first);
    const double second_length = length(second);

    const double cosine =
        dot(first, second) /
        (first_length * second_length);

    constexpr double kRadiansToDegrees =
        180.0 / 3.141592653589793238462643383279502884;

    return std::acos(clamp_cosine(cosine)) *
           kRadiansToDegrees;
}

}  // namespace

MeshQualityResult MeshQualityEvaluator::evaluate(
    const SurfaceMesh& mesh) const
{
    MeshQualityResult result{
        true,
        0,
        {},
        {}
    };

    for (const FaceMesh& face : mesh.faces) {
        const std::size_t node_count = face.nodes.size();

        for (std::size_t element_index = 0;
             element_index < face.triangles.size();
             ++element_index) {

            const MeshTriangle& triangle =
                face.triangles[element_index];

            if (triangle.node0 >= node_count ||
                triangle.node1 >= node_count ||
                triangle.node2 >= node_count) {
                result.success = false;

                std::ostringstream diagnostic;
                diagnostic
                    << "Invalid triangle connectivity at face "
                    << face.face_index
                    << ", element "
                    << element_index;

                result.diagnostic = diagnostic.str();
                result.elements.clear();
                result.element_count = 0;

                return result;
            }

            const MeshNode& p0 = face.nodes[triangle.node0];
            const MeshNode& p1 = face.nodes[triangle.node1];
            const MeshNode& p2 = face.nodes[triangle.node2];

            if (!finite_node(p0) ||
                !finite_node(p1) ||
                !finite_node(p2)) {
                result.success = false;

                std::ostringstream diagnostic;
                diagnostic
                    << "Non-finite node coordinate at face "
                    << face.face_index
                    << ", element "
                    << element_index;

                result.diagnostic = diagnostic.str();
                result.elements.clear();
                result.element_count = 0;

                return result;
            }

            const Vector3 edge0_vector = subtract(p1, p0);
            const Vector3 edge1_vector = subtract(p2, p1);
            const Vector3 edge2_vector = subtract(p0, p2);

            const Vector3 p2_minus_p0 = subtract(p2, p0);
            const Vector3 p0_minus_p1 = subtract(p0, p1);
            const Vector3 p1_minus_p2 = subtract(p1, p2);

            const double edge0 = length(edge0_vector);
            const double edge1 = length(edge1_vector);
            const double edge2 = length(edge2_vector);

            const Vector3 normal =
                cross(edge0_vector, p2_minus_p0);

            const double area =
                0.5 * length(normal);

            const double min_edge =
                std::min({edge0, edge1, edge2});

            const double max_edge =
                std::max({edge0, edge1, edge2});

            const bool degenerate =
                !std::isfinite(area) ||
                area <= kAreaTolerance ||
                min_edge <= 0.0;

            ElementQuality quality{
                face.face_index,
                element_index,
                area,
                min_edge,
                max_edge,
                0.0,
                0.0,
                0.0,
                degenerate
            };

            if (!degenerate) {
                quality.aspect_ratio =
                    max_edge / min_edge;

                const double angle0 =
                    angle_degrees(
                        edge0_vector,
                        p2_minus_p0);

                const double angle1 =
                    angle_degrees(
                        edge1_vector,
                        p0_minus_p1);

                const double angle2 =
                    angle_degrees(
                        edge2_vector,
                        p1_minus_p2);

                quality.min_angle_degrees =
                    std::min({angle0, angle1, angle2});

                quality.max_angle_degrees =
                    std::max({angle0, angle1, angle2});
            }

            result.elements.push_back(quality);
            ++result.element_count;
        }
    }

    return result;
}

}  // namespace cad2sim::mesh
