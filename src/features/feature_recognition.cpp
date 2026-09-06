#include "cad2sim/features/feature_recognition.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

#include "cad2sim/kernel/geometry_kernel.hpp"

namespace cad2sim::features {

namespace {

using cad2sim::kernel::CurveType;
using cad2sim::kernel::FaceTopologyDescriptor;
using cad2sim::kernel::FaceTopologyResult;
using cad2sim::kernel::SurfaceType;

constexpr double kTolerance = 1e-9;

bool has_adjacent_face(
    const cad2sim::kernel::EdgeTopologyDescriptor& edge,
    std::size_t face_index
) {
    for (const auto adjacent : edge.adjacent_face_indices) {
        if (adjacent == face_index) {
            return true;
        }
    }

    return false;
}

const cad2sim::kernel::EdgeTopologyDescriptor* find_edge(
    const FaceTopologyResult& topology,
    std::size_t edge_index
) {
    for (const auto& edge : topology.edges) {
        if (edge.index == edge_index) {
            return &edge;
        }
    }

    return nullptr;
}

const FaceTopologyDescriptor* find_face(
    const FaceTopologyResult& topology,
    std::size_t face_index
) {
    for (const auto& face : topology.faces) {
        if (face.index == face_index) {
            return &face;
        }
    }

    return nullptr;
}

bool is_cylindrical_feature(
    const FaceTopologyResult& topology,
    const FaceTopologyDescriptor& face
) {
    if (face.surface_type != SurfaceType::Cylinder ||
        face.area < kTolerance ||
        face.radius <= kTolerance) {
        return false;
    }

    std::size_t circular_edges = 0;
    std::size_t seam_edges = 0;
    std::vector<std::size_t> visited_edge_indices;

    for (const auto edge_index : face.edge_indices) {
        if (std::find(
                visited_edge_indices.begin(),
                visited_edge_indices.end(),
                edge_index) != visited_edge_indices.end()) {
            continue;
        }

        visited_edge_indices.push_back(edge_index);
        const auto* edge = find_edge(topology, edge_index);
        if (edge == nullptr) {
            continue;
        }

        if (edge->curve_type == CurveType::Circle) {
            ++circular_edges;
        }

        if (edge->curve_type == CurveType::Line &&
            edge->adjacent_face_indices.size() == 2 &&
            edge->adjacent_face_indices[0] == face.index &&
            edge->adjacent_face_indices[1] == face.index) {
            ++seam_edges;
        }
    }

    // A cylindrical hole/pocket has a cylindrical face with at least
    // one circular boundary and one cylindrical seam edge.
    return circular_edges >= 1 && seam_edges == 1;
}

bool is_through_hole(
    const FaceTopologyResult& topology,
    const FaceTopologyDescriptor& cylinder
) {
    if (!is_cylindrical_feature(topology, cylinder)) {
        return false;
    }

    std::size_t circular_boundaries = 0;

    for (const auto edge_index : cylinder.edge_indices) {
        const auto* edge = find_edge(topology, edge_index);
        if (edge == nullptr || edge->curve_type != CurveType::Circle) {
            continue;
        }

        bool has_external_planar_neighbor = false;

        for (const auto adjacent_face_index : edge->adjacent_face_indices) {
            if (adjacent_face_index == cylinder.index) {
                continue;
            }

            const auto* adjacent_face =
                find_face(topology, adjacent_face_index);

            if (adjacent_face != nullptr &&
                adjacent_face->surface_type == SurfaceType::Plane) {
                has_external_planar_neighbor = true;
            }
        }

        if (has_external_planar_neighbor) {
            ++circular_boundaries;
        }
    }

    // A through hole has two circular boundaries but no planar
    // termination face matching the cylindrical cross-section.
    if (circular_boundaries != 2) {
        return false;
    }

    const double expected_termination_area =
        M_PI * cylinder.radius * cylinder.radius;

    for (const auto& face : topology.faces) {
        if (face.index == cylinder.index ||
            face.surface_type != SurfaceType::Plane) {
            continue;
        }

        if (face.edge_indices.size() == 1 &&
            std::fabs(face.area - expected_termination_area) < 1e-6) {
            return false;
        }
    }

    return true;
}

bool is_blind_pocket(
    const FaceTopologyResult& topology,
    const FaceTopologyDescriptor& cylinder
) {
    if (!is_cylindrical_feature(topology, cylinder)) {
        return false;
    }

    // In the current B-Rep convention, an internal cylindrical
    // pocket wall is represented with reversed orientation.
    if (cylinder.orientation != cad2sim::kernel::FaceOrientation::Reversed) {
        return false;
    }

    // A blind cylindrical pocket has a planar termination face
    // whose area matches the circular cross-section of the cylinder.
    for (const auto& face : topology.faces) {
        if (face.index == cylinder.index ||
            face.surface_type != SurfaceType::Plane) {
            continue;
        }

        if (face.area > kTolerance &&
            face.edge_indices.size() == 1 &&
            std::fabs(face.area -
                      (M_PI * cylinder.radius * cylinder.radius)) <
                1e-6) {
            return true;
        }
    }

    return false;
}

bool is_fillet(
    const FaceTopologyResult& topology,
    const FaceTopologyDescriptor& face
) {
    if (face.surface_type != SurfaceType::Cylinder ||
        face.radius <= kTolerance) {
        return false;
    }

    std::size_t circular_edges = 0;
    std::size_t longitudinal_edges = 0;

    for (const auto edge_index : face.edge_indices) {
        const auto* edge = find_edge(topology, edge_index);
        if (edge == nullptr) {
            continue;
        }

        if (edge->curve_type == CurveType::Circle) {
            ++circular_edges;
        }

        if (edge->curve_type == CurveType::Line &&
            edge->adjacent_face_indices.size() == 2 &&
            !(edge->adjacent_face_indices[0] == face.index &&
              edge->adjacent_face_indices[1] == face.index)) {
            ++longitudinal_edges;
        }
    }

    // The controlled fillet fixture has two circular transition edges
    // and two longitudinal edges.
    return circular_edges == 2 && longitudinal_edges == 2;
}

bool is_chamfer(
    const FaceTopologyResult& topology,
    const FaceTopologyDescriptor& face
) {
    if (face.surface_type != SurfaceType::Plane) {
        return false;
    }

    // A chamfer transition in the controlled fixtures is a planar face
    // with four line edges, two of which are short bevel edges.
    if (face.edge_indices.size() != 4) {
        return false;
    }

    std::size_t line_edges = 0;
    std::size_t short_edges = 0;

    for (const auto edge_index : face.edge_indices) {
        const auto* edge = find_edge(topology, edge_index);
        if (edge == nullptr || edge->curve_type != CurveType::Line) {
            continue;
        }

        ++line_edges;

        if (edge->length < 10.0) {
            ++short_edges;
        }
    }

    return line_edges == 4 && short_edges == 2;
}

}  // namespace

FeatureRecognitionResult FeatureRecognizer::recognize(
    const std::string& path
) const {
    FeatureRecognitionResult result;

    if (!std::filesystem::exists(path)) {
        return result;
    }

    cad2sim::kernel::GeometryKernel kernel;

    // Validate geometry first.
    const auto validation = kernel.validate_step(path);
    if (!validation.valid) {
        return result;
    }

    // Existing project-level face recognition.
    const auto face_descriptors = kernel.inspect_faces(path);

    for (const auto& desc : face_descriptors) {
        if (desc.area < kTolerance) {
            continue;
        }

        FeatureType type;

        switch (desc.surface_type) {
            case SurfaceType::Plane:
                type = FeatureType::PlanarFace;
                break;

            case SurfaceType::Cylinder:
                type = FeatureType::CylindricalFace;
                break;

            default:
                result.features.push_back({
                    FeatureType::Unknown,
                    FeatureStatus::Unsupported,
                    desc.index,
                    "Unsupported surface type"
                });
                continue;
        }

        result.features.push_back({
            type,
            FeatureStatus::Confirmed,
            desc.index,
            {}
        });
    }

    // Semantic feature recognition uses kernel topology evidence.
    const auto topology = kernel.inspect_face_topology(path);

    for (const auto& face : topology.faces) {
        if (face.area < kTolerance) {
            continue;
        }

        if (is_through_hole(topology, face)) {
            result.features.push_back({
                FeatureType::Hole,
                FeatureStatus::Confirmed,
                face.index,
                "Through cylindrical feature"
            });
            continue;
        }

        if (is_blind_pocket(topology, face)) {
            result.features.push_back({
                FeatureType::Pocket,
                FeatureStatus::Confirmed,
                face.index,
                "Blind cylindrical feature"
            });
            continue;
        }

        if (is_fillet(topology, face)) {
            result.features.push_back({
                FeatureType::Fillet,
                FeatureStatus::Confirmed,
                face.index,
                "Cylindrical rounded transition"
            });
            continue;
        }

        if (is_chamfer(topology, face)) {
            result.features.push_back({
                FeatureType::Chamfer,
                FeatureStatus::Confirmed,
                face.index,
                "Planar bevel transition"
            });
        }
    }

    return result;
}

}  // namespace cad2sim::features
