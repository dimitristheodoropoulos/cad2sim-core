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

void test_circle3() {
    const cad2sim::geometry::Circle3 circle{
        cad2sim::geometry::Point3{0.0, 0.0, 0.0},
        2.0
    };

    const auto point = circle.at(0.0);

    assert_vec3_near(
        point,
        cad2sim::math::Vec3{2.0, 0.0, 0.0}
    );

    const auto quarter_point =
        circle.at(std::acos(-1.0) / 2.0);

    assert_vec3_near(
        quarter_point,
        cad2sim::math::Vec3{0.0, 2.0, 0.0}
    );
}

void test_plane_surface3() {
    const cad2sim::geometry::PlaneSurface3 surface{
        cad2sim::geometry::Point3{0.0, 0.0, 5.0}
    };

    const auto point = surface.evaluate(2.0, 3.0);

    assert_vec3_near(
        point,
        cad2sim::math::Vec3{2.0, 3.0, 5.0}
    );
}

void test_frame3() {
    const cad2sim::geometry::Frame3 frame{
        cad2sim::geometry::Point3{10.0, 20.0, 30.0},
        cad2sim::math::Vec3{0.0, 1.0, 0.0},
        cad2sim::math::Vec3{-1.0, 0.0, 0.0},
        cad2sim::math::Vec3{0.0, 0.0, 1.0}
    };

    const auto world_point =
        frame.to_world(
            cad2sim::math::Vec3{1.0, 2.0, 3.0}
        );

    assert_vec3_near(
        world_point,
        cad2sim::math::Vec3{8.0, 21.0, 33.0}
    );

    const auto local_point =
        frame.to_local(world_point);

    assert_vec3_near(
        local_point,
        cad2sim::math::Vec3{1.0, 2.0, 3.0}
    );
}

}  // namespace

int main() {
    test_circle3();
    test_plane_surface3();
    test_frame3();

    return 0;
}
