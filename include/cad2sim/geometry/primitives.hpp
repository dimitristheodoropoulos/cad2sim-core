#pragma once

#include <cmath>

#include "cad2sim/math/transform3.hpp"
#include "cad2sim/math/vec3.hpp"

namespace cad2sim::geometry {

struct Point3 {
    double x;
    double y;
    double z;
};

struct Line3 {
    Point3 origin;
    cad2sim::math::Vec3 direction;

    cad2sim::math::Vec3 at(double parameter) const {
        return cad2sim::math::Vec3{
            origin.x + parameter * direction.x,
            origin.y + parameter * direction.y,
            origin.z + parameter * direction.z
        };
    }
};

struct Plane3 {
    Point3 origin;
    cad2sim::math::Vec3 normal;

    double signed_distance(const Point3& point) const {
        const cad2sim::math::Vec3 displacement{
            point.x - origin.x,
            point.y - origin.y,
            point.z - origin.z
        };

        return cad2sim::math::dot(displacement, normal);
    }
};

struct Circle3 {
    Point3 center;
    double radius;

    cad2sim::math::Vec3 at(double angle_radians) const {
        return cad2sim::math::Vec3{
            center.x + radius * std::cos(angle_radians),
            center.y + radius * std::sin(angle_radians),
            center.z
        };
    }
};

struct PlaneSurface3 {
    Point3 origin;

    cad2sim::math::Vec3 evaluate(
        double u,
        double v
    ) const {
        return cad2sim::math::Vec3{
            origin.x + u,
            origin.y + v,
            origin.z
        };
    }
};

struct Frame3 {
    Point3 origin;
    cad2sim::math::Vec3 x_axis;
    cad2sim::math::Vec3 y_axis;
    cad2sim::math::Vec3 z_axis;

    cad2sim::math::Vec3 to_world(
        const cad2sim::math::Vec3& local_point
    ) const {
        return cad2sim::math::Vec3{
            origin.x +
                local_point.x * x_axis.x +
                local_point.y * y_axis.x +
                local_point.z * z_axis.x,

            origin.y +
                local_point.x * x_axis.y +
                local_point.y * y_axis.y +
                local_point.z * z_axis.y,

            origin.z +
                local_point.x * x_axis.z +
                local_point.y * y_axis.z +
                local_point.z * z_axis.z
        };
    }

    cad2sim::math::Vec3 to_local(
        const cad2sim::math::Vec3& world_point
    ) const {
        const cad2sim::math::Vec3 displacement{
            world_point.x - origin.x,
            world_point.y - origin.y,
            world_point.z - origin.z
        };

        return cad2sim::math::Vec3{
            cad2sim::math::dot(displacement, x_axis),
            cad2sim::math::dot(displacement, y_axis),
            cad2sim::math::dot(displacement, z_axis)
        };
    }
};

}  // namespace cad2sim::geometry
