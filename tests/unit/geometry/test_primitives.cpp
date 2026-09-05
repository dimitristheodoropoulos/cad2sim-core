#include <cassert>
#include <cmath>

#include "cad2sim/geometry/primitives.hpp"

namespace {

constexpr double kTolerance = 1e-12;

bool near(double a, double b) {
    return std::abs(a - b) <= kTolerance;
}

void assert_vec3_near(
    const cad2sim::math::Vec3& actual,
    const cad2sim::math::Vec3& expected
) {
    assert(near(actual.x, expected.x));
    assert(near(actual.y, expected.y));
    assert(near(actual.z, expected.z));
}

void test_point3() {
    const cad2sim::geometry::Point3 point{
        1.0, 2.0, 3.0
    };

    assert(near(point.x, 1.0));
    assert(near(point.y, 2.0));
    assert(near(point.z, 3.0));
}

void test_line3() {
    const cad2sim::geometry::Point3 origin{
        1.0, 2.0, 3.0
    };

    const cad2sim::math::Vec3 direction{
        0.0, 0.0, 1.0
    };

    const cad2sim::geometry::Line3 line{
        origin,
        direction
    };

    const auto point = line.at(5.0);

    assert_vec3_near(
        point,
        cad2sim::math::Vec3{1.0, 2.0, 8.0}
    );
}

void test_plane3() {
    const cad2sim::geometry::Point3 origin{
        0.0, 0.0, 5.0
    };

    const cad2sim::math::Vec3 normal{
        0.0, 0.0, 1.0
    };

    const cad2sim::geometry::Plane3 plane{
        origin,
        normal
    };

    assert(near(plane.signed_distance(
        cad2sim::geometry::Point3{0.0, 0.0, 8.0}
    ), 3.0));

    assert(near(plane.signed_distance(
        cad2sim::geometry::Point3{0.0, 0.0, 2.0}
    ), -3.0));
}

}  // namespace

int main() {
    test_point3();
    test_line3();
    test_plane3();

    return 0;
}
