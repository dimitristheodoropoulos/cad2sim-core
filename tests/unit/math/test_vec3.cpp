#include <cassert>
#include <cmath>
#include <stdexcept>

#include "cad2sim/math/vec3.hpp"

namespace {

constexpr double kTolerance = 1e-12;

bool near(double a, double b) {
    return std::abs(a - b) <= kTolerance;
}

void test_dot() {
    const cad2sim::math::Vec3 a{1.0, 2.0, 3.0};
    const cad2sim::math::Vec3 b{4.0, -5.0, 6.0};

    assert(near(cad2sim::math::dot(a, b), 12.0));
}

void test_cross() {
    const cad2sim::math::Vec3 a{1.0, 0.0, 0.0};
    const cad2sim::math::Vec3 b{0.0, 1.0, 0.0};

    const auto result = cad2sim::math::cross(a, b);

    assert(near(result.x, 0.0));
    assert(near(result.y, 0.0));
    assert(near(result.z, 1.0));
}

void test_norm() {
    const cad2sim::math::Vec3 v{3.0, 4.0, 0.0};

    assert(near(cad2sim::math::norm(v), 5.0));
}

void test_normalized() {
    const cad2sim::math::Vec3 v{3.0, 4.0, 0.0};

    const auto result = cad2sim::math::normalized(v);

    assert(near(result.x, 0.6));
    assert(near(result.y, 0.8));
    assert(near(result.z, 0.0));
    assert(near(cad2sim::math::norm(result), 1.0));
}

void test_distance() {
    const cad2sim::math::Vec3 a{1.0, 2.0, 3.0};
    const cad2sim::math::Vec3 b{4.0, 6.0, 3.0};

    assert(near(cad2sim::math::distance(a, b), 5.0));
}

void test_project() {
    const cad2sim::math::Vec3 vector{3.0, 4.0, 0.0};
    const cad2sim::math::Vec3 onto{1.0, 0.0, 0.0};

    const auto result = cad2sim::math::project(vector, onto);

    assert(near(result.x, 3.0));
    assert(near(result.y, 0.0));
    assert(near(result.z, 0.0));
}

void test_project_zero_vector_throws() {
    const cad2sim::math::Vec3 vector{1.0, 2.0, 3.0};
    const cad2sim::math::Vec3 zero{0.0, 0.0, 0.0};

    bool threw = false;

    try {
        (void)cad2sim::math::project(vector, zero);
    } catch (const std::invalid_argument&) {
        threw = true;
    }

    assert(threw);
}

}  // namespace

int main() {
    test_dot();
    test_cross();
    test_norm();
    test_normalized();
    test_distance();
    test_project();
    test_project_zero_vector_throws();

    return 0;
}
