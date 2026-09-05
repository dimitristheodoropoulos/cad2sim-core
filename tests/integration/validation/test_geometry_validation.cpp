#include <cassert>
#include <filesystem>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <gp_Pnt.hxx>

#include "cad2sim/kernel/geometry_kernel.hpp"
#include "geometry_validation.hpp"

int main() {
    const std::filesystem::path fixture =
        std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
        "tests" /
        "fixtures" /
        "step" /
        "screw.step";

    assert(std::filesystem::exists(fixture));

    cad2sim::kernel::GeometryKernel kernel;

    const auto valid_result = kernel.validate_step(fixture.string());

    assert(valid_result.valid);
    assert(valid_result.diagnostic.empty());

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

    const auto invalid_result =
        cad2sim::kernel::detail::validate_shape(invalid_solid);

    assert(!invalid_result.valid);
    assert(
        invalid_result.diagnostic ==
        "Geometry validation failed: B-Rep shape is invalid"
    );

    return 0;
}
