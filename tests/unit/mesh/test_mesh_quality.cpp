#include <cassert>
#include <cmath>
#include <cstddef>
#include <iostream>

#include "cad2sim/mesh/mesh_quality.hpp"

namespace {

using cad2sim::mesh::MeshNode;
using cad2sim::mesh::MeshTriangle;
using cad2sim::mesh::SurfaceMesh;
using cad2sim::mesh::MeshQualityEvaluator;
using cad2sim::mesh::MeshQualityResult;

MeshNode node(double x, double y, double z)
{
    return MeshNode{x, y, z};
}

SurfaceMesh single_triangle(
    MeshNode a,
    MeshNode b,
    MeshNode c)
{
    SurfaceMesh mesh;

    mesh.faces.push_back({
        0,
        {a, b, c},
        {{0, 1, 2}}
    });

    return mesh;
}

void test_equilateral_triangle()
{
    const double h = std::sqrt(3.0) / 2.0;

    const SurfaceMesh mesh = single_triangle(
        node(0.0, 0.0, 0.0),
        node(1.0, 0.0, 0.0),
        node(0.5, h, 0.0));

    const MeshQualityResult result =
        MeshQualityEvaluator{}.evaluate(mesh);

    assert(result.success);
    assert(result.element_count == 1);

    assert(result.elements.size() == 1);

    const auto& quality = result.elements[0];

    assert(std::abs(quality.area - std::sqrt(3.0) / 4.0) < 1e-12);
    assert(std::abs(quality.min_edge_length - 1.0) < 1e-12);
    assert(std::abs(quality.max_edge_length - 1.0) < 1e-12);
    assert(std::abs(quality.aspect_ratio - 1.0) < 1e-12);
    assert(std::abs(quality.min_angle_degrees - 60.0) < 1e-10);
    assert(std::abs(quality.max_angle_degrees - 60.0) < 1e-10);
    assert(!quality.degenerate);
}

void test_right_isosceles_triangle()
{
    const SurfaceMesh mesh = single_triangle(
        node(0.0, 0.0, 0.0),
        node(1.0, 0.0, 0.0),
        node(0.0, 1.0, 0.0));

    const MeshQualityResult result =
        MeshQualityEvaluator{}.evaluate(mesh);

    assert(result.success);
    assert(result.element_count == 1);

    const auto& quality = result.elements[0];

    assert(std::abs(quality.area - 0.5) < 1e-12);
    assert(std::abs(quality.min_edge_length - 1.0) < 1e-12);
    assert(std::abs(quality.max_edge_length - std::sqrt(2.0)) < 1e-12);
    assert(std::abs(quality.aspect_ratio - std::sqrt(2.0)) < 1e-12);
    assert(std::abs(quality.min_angle_degrees - 45.0) < 1e-10);
    assert(std::abs(quality.max_angle_degrees - 90.0) < 1e-10);
    assert(!quality.degenerate);
}

void test_degraded_high_aspect_ratio_triangle()
{
    const SurfaceMesh mesh = single_triangle(
        node(0.0, 0.0, 0.0),
        node(10.0, 0.0, 0.0),
        node(0.0, 0.1, 0.0));

    const MeshQualityResult result =
        MeshQualityEvaluator{}.evaluate(mesh);

    assert(result.success);
    assert(result.element_count == 1);

    const auto& quality = result.elements[0];

    assert(quality.aspect_ratio > 50.0);
    assert(quality.min_angle_degrees < 10.0);
    assert(!quality.degenerate);
}

void test_zero_area_triangle()
{
    const SurfaceMesh mesh = single_triangle(
        node(0.0, 0.0, 0.0),
        node(1.0, 0.0, 0.0),
        node(2.0, 0.0, 0.0));

    const MeshQualityResult result =
        MeshQualityEvaluator{}.evaluate(mesh);

    assert(result.success);
    assert(result.element_count == 1);

    const auto& quality = result.elements[0];

    assert(quality.degenerate);
    assert(std::abs(quality.area) < 1e-12);
}

void test_invalid_connectivity()
{
    SurfaceMesh mesh = single_triangle(
        node(0.0, 0.0, 0.0),
        node(1.0, 0.0, 0.0),
        node(0.0, 1.0, 0.0));

    mesh.faces[0].triangles[0] = {0, 1, 99};

    const MeshQualityResult result =
        MeshQualityEvaluator{}.evaluate(mesh);

    assert(!result.success);
    assert(!result.diagnostic.empty());
}

}  // namespace

int main()
{
    test_equilateral_triangle();
    test_right_isosceles_triangle();
    test_degraded_high_aspect_ratio_triangle();
    test_zero_area_triangle();
    test_invalid_connectivity();

    std::cout << "[PASS] mesh quality unit contract\n";
    return 0;
}
