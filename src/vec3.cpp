#include "cad2sim/math/vec3.hpp"

#include <cmath>
#include <stdexcept>

namespace cad2sim::math {

double dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x +
           a.y * b.y +
           a.z * b.z;
}

Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3{
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

double norm(const Vec3& v) {
    return std::sqrt(dot(v, v));
}

Vec3 normalized(const Vec3& v) {
    const double length = norm(v);

    if (length == 0.0) {
        throw std::invalid_argument("cannot normalize zero-length vector");
    }

    return Vec3{
        v.x / length,
        v.y / length,
        v.z / length
    };
}

double distance(const Vec3& a, const Vec3& b) {
    return norm(Vec3{
        a.x - b.x,
        a.y - b.y,
        a.z - b.z
    });
}

Vec3 project(const Vec3& vector, const Vec3& onto) {
    const double denominator = dot(onto, onto);

    if (denominator == 0.0) {
        throw std::invalid_argument(
            "cannot project onto zero-length vector"
        );
    }

    const double scale = dot(vector, onto) / denominator;

    return Vec3{
        scale * onto.x,
        scale * onto.y,
        scale * onto.z
    };
}

}  // namespace cad2sim::math
