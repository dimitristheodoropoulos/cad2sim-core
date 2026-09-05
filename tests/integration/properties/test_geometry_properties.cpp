#include <cassert>
#include <cmath>
#include <filesystem>

#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>
#include <TopoDS_Shape.hxx>

#include "cad2sim/kernel/geometry_kernel.hpp"

namespace {

constexpr double kTolerance = 1.0e-6;

bool nearly_equal(double a, double b) {
    return std::abs(a - b) <= kTolerance;
}

bool is_axis_normal(const cad2sim::kernel::Vector3& normal) {
    const double length =
        std::sqrt(
            normal.x * normal.x +
            normal.y * normal.y +
            normal.z * normal.z
        );

    if (!nearly_equal(length, 1.0)) {
        return false;
    }

    const int non_zero =
        (std::abs(normal.x) > kTolerance ? 1 : 0) +
        (std::abs(normal.y) > kTolerance ? 1 : 0) +
        (std::abs(normal.z) > kTolerance ? 1 : 0);

    return non_zero == 1;
}

}  // namespace

int main() {
    cad2sim::kernel::GeometryKernel kernel;

    /*
     * Analytical geometry:
     * 10 x 20 x 30 box.
     */
    const TopoDS_Shape box =
        BRepPrimAPI_MakeBox(10.0, 20.0, 30.0).Shape();

    assert(!box.IsNull());

    const std::filesystem::path analytical_step =
        std::filesystem::temp_directory_path() /
        "cad2sim_req007_box.step";

    STEPControl_Writer writer;

    const IFSelect_ReturnStatus transfer_status =
        writer.Transfer(
            box,
            STEPControl_AsIs
        );

    assert(transfer_status == IFSelect_RetDone);

    const IFSelect_ReturnStatus write_status =
        writer.Write(analytical_step.string().c_str());

    assert(write_status == IFSelect_RetDone);
    assert(std::filesystem::exists(analytical_step));

    const auto box_properties =
        kernel.inspect_geometry_properties(
            analytical_step.string()
        );

    assert(nearly_equal(
        box_properties.bounding_box.min_x,
        0.0
    ));
    assert(nearly_equal(
        box_properties.bounding_box.min_y,
        0.0
    ));
    assert(nearly_equal(
        box_properties.bounding_box.min_z,
        0.0
    ));
    assert(nearly_equal(
        box_properties.bounding_box.max_x,
        10.0
    ));
    assert(nearly_equal(
        box_properties.bounding_box.max_y,
        20.0
    ));
    assert(nearly_equal(
        box_properties.bounding_box.max_z,
        30.0
    ));

    assert(nearly_equal(
        box_properties.volume,
        6000.0
    ));

    assert(nearly_equal(
        box_properties.centroid.x,
        5.0
    ));
    assert(nearly_equal(
        box_properties.centroid.y,
        10.0
    ));
    assert(nearly_equal(
        box_properties.centroid.z,
        15.0
    ));

    assert(box_properties.faces.size() == 6);
    assert(box_properties.edges.size() == 12);

    int area_600 = 0;
    int area_300 = 0;
    int area_200 = 0;

    for (const auto& face : box_properties.faces) {
        assert(face.area > 0.0);
        assert(is_axis_normal(face.normal));

        if (nearly_equal(face.area, 600.0)) {
            ++area_600;
        } else if (nearly_equal(face.area, 300.0)) {
            ++area_300;
        } else if (nearly_equal(face.area, 200.0)) {
            ++area_200;
        } else {
            assert(false);
        }
    }

    assert(area_600 == 2);
    assert(area_300 == 2);
    assert(area_200 == 2);

    int length_10 = 0;
    int length_20 = 0;
    int length_30 = 0;

    for (const auto& edge : box_properties.edges) {
        assert(edge.length > 0.0);

        if (nearly_equal(edge.length, 10.0)) {
            ++length_10;
        } else if (nearly_equal(edge.length, 20.0)) {
            ++length_20;
        } else if (nearly_equal(edge.length, 30.0)) {
            ++length_30;
        } else {
            assert(false);
        }
    }

    assert(length_10 == 4);
    assert(length_20 == 4);
    assert(length_30 == 4);

    std::filesystem::remove(analytical_step);

    /*
     * Real CAD integration:
     * existing representative STEP fixture.
     */
    const std::filesystem::path fixture =
        std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
        "tests" /
        "fixtures" /
        "step" /
        "screw.step";

    assert(std::filesystem::exists(fixture));

    const auto step_properties =
        kernel.inspect_geometry_properties(
            fixture.string()
        );

    assert(step_properties.bounding_box.max_x >
           step_properties.bounding_box.min_x);
    assert(step_properties.bounding_box.max_y >
           step_properties.bounding_box.min_y);
    assert(step_properties.bounding_box.max_z >
           step_properties.bounding_box.min_z);

    assert(step_properties.volume > 0.0);

    assert(step_properties.faces.size() > 0);
    assert(step_properties.edges.size() > 0);

    for (const auto& face : step_properties.faces) {
        assert(face.area > 0.0);

        const double normal_length =
            std::sqrt(
                face.normal.x * face.normal.x +
                face.normal.y * face.normal.y +
                face.normal.z * face.normal.z
            );

        assert(nearly_equal(normal_length, 1.0));
    }

    for (const auto& edge : step_properties.edges) {
        assert(edge.length > 0.0);
    }

    return 0;
}
