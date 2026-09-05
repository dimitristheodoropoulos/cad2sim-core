#include "cad2sim/math/transform3.hpp"

#include <cmath>
#include <stdexcept>

namespace cad2sim::math {

Transform3::Transform3(double matrix[4][4]) {
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            matrix_[row][column] = matrix[row][column];
        }
    }
}

Transform3 Transform3::identity() {
    double matrix[4][4]{};

    for (int i = 0; i < 4; ++i) {
        matrix[i][i] = 1.0;
    }

    return Transform3(matrix);
}

Transform3 Transform3::translation(const Vec3& offset) {
    double matrix[4][4]{};

    for (int i = 0; i < 4; ++i) {
        matrix[i][i] = 1.0;
    }

    matrix[0][3] = offset.x;
    matrix[1][3] = offset.y;
    matrix[2][3] = offset.z;

    return Transform3(matrix);
}

Transform3 Transform3::rotation_z(double angle_radians) {
    const double c = std::cos(angle_radians);
    const double s = std::sin(angle_radians);

    double matrix[4][4]{};

    matrix[0][0] = c;
    matrix[0][1] = -s;
    matrix[1][0] = s;
    matrix[1][1] = c;
    matrix[2][2] = 1.0;
    matrix[3][3] = 1.0;

    return Transform3(matrix);
}

Vec3 Transform3::apply(const Vec3& point) const {
    return Vec3{
        matrix_[0][0] * point.x +
            matrix_[0][1] * point.y +
            matrix_[0][2] * point.z +
            matrix_[0][3],

        matrix_[1][0] * point.x +
            matrix_[1][1] * point.y +
            matrix_[1][2] * point.z +
            matrix_[1][3],

        matrix_[2][0] * point.x +
            matrix_[2][1] * point.y +
            matrix_[2][2] * point.z +
            matrix_[2][3]
    };
}

Transform3 Transform3::inverse() const {
    // For rigid transforms represented by this class,
    // the upper-left 3x3 matrix is a rotation matrix.
    double inverse[4][4]{};

    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            inverse[row][column] = matrix_[column][row];
        }
    }

    inverse[3][3] = 1.0;

    const Vec3 translation_vector{
        matrix_[0][3],
        matrix_[1][3],
        matrix_[2][3]
    };

    inverse[0][3] = -(
        inverse[0][0] * translation_vector.x +
        inverse[0][1] * translation_vector.y +
        inverse[0][2] * translation_vector.z
    );

    inverse[1][3] = -(
        inverse[1][0] * translation_vector.x +
        inverse[1][1] * translation_vector.y +
        inverse[1][2] * translation_vector.z
    );

    inverse[2][3] = -(
        inverse[2][0] * translation_vector.x +
        inverse[2][1] * translation_vector.y +
        inverse[2][2] * translation_vector.z
    );

    return Transform3(inverse);
}

Transform3 Transform3::operator*(const Transform3& other) const {
    double result[4][4]{};

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            for (int k = 0; k < 4; ++k) {
                result[row][column] +=
                    matrix_[row][k] * other.matrix_[k][column];
            }
        }
    }

    return Transform3(result);
}

}  // namespace cad2sim::math
