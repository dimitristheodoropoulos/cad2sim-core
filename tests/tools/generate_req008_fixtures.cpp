#include <cassert>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <string>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Line.hxx>
#include <STEPControl_Writer.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Vertex.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>

namespace {

constexpr double kTol = 1e-7;

const std::filesystem::path output_dir =
    std::filesystem::path(CAD2SIM_TEST_SOURCE_DIR) /
    "tests" / "fixtures" / "step";

bool near(double a, double b) {
    return std::abs(a - b) <= kTol;
}

bool near_point(const gp_Pnt& a, const gp_Pnt& b) {
    return a.Distance(b) <= kTol;
}

TopoDS_Edge find_edge(
    const TopoDS_Shape& shape,
    const gp_Pnt& expected_a,
    const gp_Pnt& expected_b
) {
    for (TopExp_Explorer explorer(shape, TopAbs_EDGE);
         explorer.More();
         explorer.Next()) {

        const TopoDS_Edge edge =
            TopoDS::Edge(explorer.Current());

        TopoDS_Vertex first;
        TopoDS_Vertex last;
        TopExp::Vertices(edge, first, last);

        if (first.IsNull() || last.IsNull()) {
            continue;
        }

        const gp_Pnt p1 = BRep_Tool::Pnt(first);
        const gp_Pnt p2 = BRep_Tool::Pnt(last);

        const bool endpoints_match =
            (near_point(p1, expected_a) &&
             near_point(p2, expected_b)) ||
            (near_point(p1, expected_b) &&
             near_point(p2, expected_a));

        if (!endpoints_match) {
            continue;
        }

        Standard_Real first_param = 0.0;
        Standard_Real last_param = 0.0;

        const Handle(Geom_Curve) curve =
            BRep_Tool::Curve(edge, first_param, last_param);

        if (curve.IsNull()) {
            continue;
        }

        const Handle(Geom_Line) line =
            Handle(Geom_Line)::DownCast(curve);

        if (line.IsNull()) {
            continue;
        }

        const gp_Vec endpoint_vector(expected_a, expected_b);

        if (!near(endpoint_vector.Magnitude(), 100.0)) {
            continue;
        }

        return edge;
    }

    return TopoDS_Edge();
}

void validate_shape(
    const TopoDS_Shape& shape,
    const std::string& name
) {
    assert(!shape.IsNull());

    BRepCheck_Analyzer analyzer(shape);
    if (!analyzer.IsValid()) {
        std::cerr << "ERROR: invalid generated shape: "
                  << name << '\n';
        std::exit(1);
    }
}

void write_step(
    const TopoDS_Shape& shape,
    const std::string& filename
) {
    validate_shape(shape, filename);

    const auto path = output_dir / filename;

    STEPControl_Writer writer;

    const IFSelect_ReturnStatus transfer_status =
        writer.Transfer(shape, STEPControl_AsIs);

    if (transfer_status != IFSelect_RetDone) {
        std::cerr << "ERROR: STEP transfer failed: "
                  << filename << '\n';
        std::exit(1);
    }

    const IFSelect_ReturnStatus write_status =
        writer.Write(path.c_str());

    if (write_status != IFSelect_RetDone) {
        std::cerr << "ERROR: STEP write failed: "
                  << filename << '\n';
        std::exit(1);
    }

    if (!std::filesystem::exists(path) ||
        std::filesystem::file_size(path) == 0) {
        std::cerr << "ERROR: STEP file missing/empty: "
                  << filename << '\n';
        std::exit(1);
    }

    std::cout << "[PASS] " << filename
              << " (" << std::filesystem::file_size(path)
              << " bytes)\n";
}

TopoDS_Shape make_base_box() {
    return BRepPrimAPI_MakeBox(
        100.0,
        60.0,
        30.0
    ).Shape();
}

TopoDS_Shape make_through_hole() {
    const TopoDS_Shape box = make_base_box();

    const TopoDS_Shape cylinder =
        BRepPrimAPI_MakeCylinder(
            10.0,
            30.0
        ).Shape();

    // Move the cylinder axis from the origin to (50,30,0).
    gp_Trsf transform;
    transform.SetTranslation(
        gp_Vec(50.0, 30.0, 0.0)
    );

    TopoDS_Shape positioned_cylinder =
        cylinder.Moved(transform);

    BRepAlgoAPI_Cut cut(box, positioned_cylinder);
    cut.Build();

    if (!cut.IsDone()) {
        std::cerr << "ERROR: HOLE-001 boolean cut failed\n";
        std::exit(1);
    }

    return cut.Shape();
}

TopoDS_Shape make_blind_pocket() {
    const TopoDS_Shape box = make_base_box();

    const TopoDS_Shape cylinder =
        BRepPrimAPI_MakeCylinder(
            10.0,
            15.0
        ).Shape();

    // The pocket starts at the top surface and extends
    // 15 mm into the 30 mm block.
    gp_Trsf transform;
    transform.SetTranslation(
        gp_Vec(50.0, 30.0, 15.0)
    );

    TopoDS_Shape positioned_cylinder =
        cylinder.Moved(transform);

    BRepAlgoAPI_Cut cut(box, positioned_cylinder);
    cut.Build();

    if (!cut.IsDone()) {
        std::cerr << "ERROR: POCKET-001 boolean cut failed\n";
        std::exit(1);
    }

    return cut.Shape();
}

TopoDS_Shape make_fillet() {
    const TopoDS_Shape box = make_base_box();

    const TopoDS_Edge target =
        find_edge(
            box,
            gp_Pnt(0.0, 0.0, 0.0),
            gp_Pnt(100.0, 0.0, 0.0)
        );

    if (target.IsNull()) {
        std::cerr << "ERROR: FILLET-001 target edge not found\n";
        std::exit(1);
    }

    BRepFilletAPI_MakeFillet fillet(box);
    fillet.Add(5.0, target);
    fillet.Build();

    if (!fillet.IsDone()) {
        std::cerr << "ERROR: FILLET-001 fillet construction failed\n";
        std::exit(1);
    }

    return fillet.Shape();
}

TopoDS_Shape make_chamfer() {
    const TopoDS_Shape box = make_base_box();

    const TopoDS_Edge target =
        find_edge(
            box,
            gp_Pnt(0.0, 0.0, 0.0),
            gp_Pnt(100.0, 0.0, 0.0)
        );

    if (target.IsNull()) {
        std::cerr << "ERROR: CHAMFER-001 target edge not found\n";
        std::exit(1);
    }

    BRepFilletAPI_MakeChamfer chamfer(box);
    chamfer.Add(5.0, target);
    chamfer.Build();

    if (!chamfer.IsDone()) {
        std::cerr << "ERROR: CHAMFER-001 chamfer construction failed\n";
        std::exit(1);
    }

    return chamfer.Shape();
}

TopoDS_Shape make_external_boss() {
    // Negative control: an external cylindrical boss must not be
    // interpreted as an internal Hole.

    const TopoDS_Shape box = make_base_box();

    // The cylinder overlaps the top 5 mm of the box and extends
    // 15 mm above it. This produces one fused solid while retaining
    // a clearly external cylindrical boss.
    const TopoDS_Shape boss =
        BRepPrimAPI_MakeCylinder(
            10.0,
            20.0
        ).Shape();

    gp_Trsf transform;
    transform.SetTranslation(
        gp_Vec(50.0, 30.0, 25.0)
    );

    const TopoDS_Shape positioned_boss =
        boss.Moved(transform);

    BRepAlgoAPI_Fuse fuse(box, positioned_boss);
    fuse.Build();

    if (!fuse.IsDone()) {
        std::cerr << "ERROR: NEG-001 external boss fuse failed\n";
        std::exit(1);
    }

    return fuse.Shape();
}

TopoDS_Shape make_unsupported_surface() {
    // Negative control: a spherical surface is intentionally outside
    // the currently supported Plane/Cylinder surface set.
    return BRepPrimAPI_MakeSphere(
        gp_Pnt(50.0, 30.0, 15.0),
        10.0
    ).Shape();
}

}  // namespace

int main() {
    std::filesystem::create_directories(output_dir);

    std::cout
        << "============================================================\n"
        << "=== REQ-008 DETERMINISTIC FIXTURE GENERATOR ===\n"
        << "============================================================\n";

    write_step(
        make_through_hole(),
        "hole-001.step"
    );

    write_step(
        make_blind_pocket(),
        "pocket-001.step"
    );

    write_step(
        make_fillet(),
        "fillet-001.step"
    );

    write_step(
        make_chamfer(),
        "chamfer-001.step"
    );

    write_step(
        make_external_boss(),
        "neg-001-external-boss.step"
    );

    write_step(
        make_base_box(),
        "neg-002-sharp-edge.step"
    );

    write_step(
        make_base_box(),
        "neg-003-plain-planar.step"
    );

    write_step(
        make_unsupported_surface(),
        "neg-005-unsupported-surface.step"
    );

    std::cout
        << "============================================================\n"
        << "=== GENERATION COMPLETE ===\n"
        << "============================================================\n";

    return 0;
}
