#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>

#include "cad2sim/engineering/engineering_model.hpp"
#include "cad2sim/kernel/geometry_kernel.hpp"
#include "cad2sim/mesh/surface_mesher.hpp"
#include "geometry_validation.hpp"

namespace {

std::filesystem::path fixture(const char* name) {
    return std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
           "tests" / "fixtures" / "step" / name;
}

void test_missing_step_file() {
    cad2sim::kernel::GeometryKernel kernel;

    const auto missing =
        std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
        "tests" / "fixtures" / "step" /
        "this_file_does_not_exist.step";

    const auto result =
        kernel.validate_step(missing.string());

    assert(!result.valid);
    assert(
        result.diagnostic ==
        "Geometry validation failed: STEP file does not exist: " +
        missing.string()
    );
}

void test_malformed_step_file() {
    cad2sim::kernel::GeometryKernel kernel;

    const auto malformed =
        std::filesystem::temp_directory_path() /
        "cad2sim_req016_malformed.step";

    {
        std::ofstream output(malformed);
        assert(output.is_open());
        output << "THIS IS NOT A VALID STEP FILE\n";
    }

    const auto result =
        kernel.validate_step(malformed.string());

    assert(!result.valid);
    assert(!result.diagnostic.empty());

    std::filesystem::remove(malformed);
}

void test_invalid_brep() {
    BRepBuilderAPI_MakePolygon polygon;
    polygon.Add(gp_Pnt(0.0, 0.0, 0.0));
    polygon.Add(gp_Pnt(10.0, 0.0, 0.0));
    polygon.Add(gp_Pnt(10.0, 10.0, 0.0));
    polygon.Add(gp_Pnt(0.0, 10.0, 0.0));
    polygon.Close();

    assert(polygon.IsDone());

    const auto face =
        BRepBuilderAPI_MakeFace(polygon.Wire()).Face();

    assert(!face.IsNull());

    TopoDS_Shell open_shell;
    TopoDS_Builder builder;
    builder.MakeShell(open_shell);
    builder.Add(open_shell, face);

    const auto invalid_solid =
        BRepBuilderAPI_MakeSolid(open_shell).Solid();

    assert(!invalid_solid.IsNull());

    const auto result =
        cad2sim::kernel::detail::validate_shape(invalid_solid);

    assert(!result.valid);
    assert(
        result.diagnostic ==
        "Geometry validation failed: B-Rep shape is invalid"
    );
}

void test_invalid_linear_deflection() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::mesh::SurfaceMesher mesher;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);
    assert(loaded.shape.valid());

    cad2sim::mesh::SurfaceMeshParameters parameters;
    parameters.linear_deflection = 0.0;
    parameters.angular_deflection = 0.5;

    const auto result =
        mesher.mesh(
            loaded.shape,
            parameters
        );

    assert(!result.success);
    assert(
        result.diagnostic ==
        "Surface meshing failed: linear deflection must be finite and positive"
    );
}

void test_invalid_angular_deflection() {
    cad2sim::kernel::GeometryKernel kernel;
    cad2sim::mesh::SurfaceMesher mesher;

    const auto loaded =
        kernel.load_validated_shape(
            fixture("screw.step").string()
        );

    assert(loaded.success);
    assert(loaded.shape.valid());

    cad2sim::mesh::SurfaceMeshParameters parameters;
    parameters.linear_deflection = 0.1;
    parameters.angular_deflection = std::nan("");

    const auto result =
        mesher.mesh(
            loaded.shape,
            parameters
        );

    assert(!result.success);
    assert(
        result.diagnostic ==
        "Surface meshing failed: angular deflection must be finite and positive"
    );
}

void test_invalid_validated_shape() {
    cad2sim::mesh::SurfaceMesher mesher;

    cad2sim::kernel::ValidatedShape invalid_shape;

    const auto result =
        mesher.mesh(invalid_shape);

    assert(!result.success);
    assert(
        result.diagnostic ==
        "Surface meshing failed: validated shape is invalid"
    );
}

void test_invalid_engineering_model() {
    cad2sim::engineering::EngineeringModel model;

    model.regions.push_back({
        0,
        {
            cad2sim::engineering::RegionEntityType::Face,
            0
        }
    });

    model.material_assignments.push_back({
        99,
        "steel"
    });

    cad2sim::engineering::EngineeringModelValidator validator;

    const auto result =
        validator.validate(model);

    assert(!result.success);
    assert(
        result.diagnostic ==
        "Engineering model validation failed: material assignment references unknown region 99"
    );
}

}  // namespace

int main() {
    test_missing_step_file();
    test_malformed_step_file();
    test_invalid_brep();
    test_invalid_linear_deflection();
    test_invalid_angular_deflection();
    test_invalid_validated_shape();
    test_invalid_engineering_model();

    return 0;
}
