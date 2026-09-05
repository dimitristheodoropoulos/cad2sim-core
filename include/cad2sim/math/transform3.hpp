#pragma once

#include "cad2sim/math/vec3.hpp"

namespace cad2sim::math {

class Transform3 {
public:
    static Transform3 identity();

    static Transform3 translation(const Vec3& offset);

    static Transform3 rotation_z(double angle_radians);

    Vec3 apply(const Vec3& point) const;

    Transform3 inverse() const;

    Transform3 operator*(const Transform3& other) const;

private:
    double matrix_[4][4]{};

    explicit Transform3(double matrix[4][4]);
};

}  // namespace cad2sim::math
