#pragma once

namespace cad2sim::math {

struct Vec3 {
    double x;
    double y;
    double z;
};

double dot(const Vec3& a, const Vec3& b);

Vec3 cross(const Vec3& a, const Vec3& b);

double norm(const Vec3& v);

Vec3 normalized(const Vec3& v);

double distance(const Vec3& a, const Vec3& b);

Vec3 project(const Vec3& vector, const Vec3& onto);

}  // namespace cad2sim::math
