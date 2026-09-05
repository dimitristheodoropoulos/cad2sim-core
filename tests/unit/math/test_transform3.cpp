#include <cassert>
#include <cmath>

#include "cad2sim/math/transform3.hpp"
#include "cad2sim/math/vec3.hpp"

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

void test_translation() {
    const cad2sim::math::Vec3 translation{10.0, -2.0, 5.0};
    const auto transform =
        cad2sim::math::Transform3::translation(translation);

    const cad2sim::math::Vec3 point{1.0, 2.0, 3.0};

    const auto result = transform.apply(point);

    assert_vec3_near(
        result,
        cad2sim::math::Vec3{11.0, 0.0, 8.0}
    );
}

void test_rotation_z() {
    const auto transform =
        cad2sim::math::Transform3::rotation_z(
            std::acos(-1.0) / 2.0
        );

    const cad2sim::math::Vec3 point{1.0, 0.0, 0.0};

    const auto result = transform.apply(point);

    assert_vec3_near(
        result,
        cad2sim::math::Vec3{0.0, 1.0, 0.0}
    );
}

void test_transform_round_trip() {
    const auto rotation =
        cad2sim::math::Transform3::rotation_z(
            std::acos(-1.0) / 3.0
        );

    const auto translation =
        cad2sim::math::Transform3::translation(
            cad2sim::math::Vec3{4.0, -3.0, 2.0}
        );

    const auto transform = translation * rotation;

    const cad2sim::math::Vec3 original{2.0, 5.0, -1.0};

    const auto transformed = transform.apply(original);
    const auto recovered = transform.inverse().apply(transformed);

    assert_vec3_near(recovered, original);
}

}  // namespace

int main() {
    test_translation();
    test_rotation_z();
    test_transform_round_trip();

    return 0;
}
