#include "cad2sim/mesh/surface_mesher.hpp"

#include <cmath>
#include <utility>

#include <BRepMesh_IncrementalMesh.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopLoc_Location.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include "../kernel/detail/validated_shape_data.hpp"

namespace cad2sim::mesh {

namespace {

bool valid_parameter(double value) {
    return std::isfinite(value) && value > 0.0;
}

}  // namespace

SurfaceMeshResult SurfaceMesher::mesh(
    const kernel::ValidatedShape& shape,
    const SurfaceMeshParameters& parameters
) const {
    if (!shape.valid()) {
        return {
            false,
            {},
            "Surface meshing failed: validated shape is invalid"
        };
    }

    if (!valid_parameter(parameters.linear_deflection)) {
        return {
            false,
            {},
            "Surface meshing failed: linear deflection must be finite and positive"
        };
    }

    if (!valid_parameter(parameters.angular_deflection)) {
        return {
            false,
            {},
            "Surface meshing failed: angular deflection must be finite and positive"
        };
    }

    const TopoDS_Shape& occt_shape =
        shape.data_->shape();

    BRepMesh_IncrementalMesh mesher(
        occt_shape,
        parameters.linear_deflection,
        false,
        parameters.angular_deflection,
        false
    );

    if (!mesher.IsDone()) {
        return {
            false,
            {},
            "Surface meshing failed: OCCT meshing did not complete"
        };
    }

    SurfaceMesh result;

    std::size_t face_index = 0;

    for (TopExp_Explorer explorer(
             occt_shape,
             TopAbs_FACE
         );
         explorer.More();
         explorer.Next(), ++face_index) {

        const TopoDS_Face face =
            TopoDS::Face(explorer.Current());

        TopLoc_Location location;

        const Handle(Poly_Triangulation) triangulation =
            BRep_Tool::Triangulation(
                face,
                location
            );

        if (triangulation.IsNull()) {
            return {
                false,
                {},
                "Surface meshing failed: face has no triangulation"
            };
        }

        FaceMesh face_mesh;
        face_mesh.face_index = face_index;

        const Standard_Integer node_count =
            triangulation->NbNodes();

        face_mesh.nodes.reserve(
            static_cast<std::size_t>(node_count)
        );

        for (Standard_Integer i = 1;
             i <= node_count;
             ++i) {

            gp_Pnt point =
                triangulation->Node(i);

            point.Transform(
                location.Transformation()
            );

            if (!std::isfinite(point.X()) ||
                !std::isfinite(point.Y()) ||
                !std::isfinite(point.Z())) {
                return {
                    false,
                    {},
                    "Surface meshing failed: non-finite mesh node coordinate"
                };
            }

            face_mesh.nodes.push_back({
                point.X(),
                point.Y(),
                point.Z()
            });
        }

        const Standard_Integer triangle_count =
            triangulation->NbTriangles();

        face_mesh.triangles.reserve(
            static_cast<std::size_t>(triangle_count)
        );

        for (Standard_Integer i = 1;
             i <= triangle_count;
             ++i) {

            Standard_Integer n1;
            Standard_Integer n2;
            Standard_Integer n3;

            triangulation->Triangle(i).Get(
                n1,
                n2,
                n3
            );

            if (n1 <= 0 ||
                n2 <= 0 ||
                n3 <= 0 ||
                n1 > node_count ||
                n2 > node_count ||
                n3 > node_count) {
                return {
                    false,
                    {},
                    "Surface meshing failed: triangle contains invalid node index"
                };
            }

            face_mesh.triangles.push_back({
                static_cast<std::size_t>(n1 - 1),
                static_cast<std::size_t>(n2 - 1),
                static_cast<std::size_t>(n3 - 1)
            });
        }

        if (face_mesh.nodes.empty() ||
            face_mesh.triangles.empty()) {
            return {
                false,
                {},
                "Surface meshing failed: face produced empty triangulation"
            };
        }

        result.faces.push_back(
            std::move(face_mesh)
        );
    }

    if (result.faces.empty()) {
        return {
            false,
            {},
            "Surface meshing failed: shape contains no faces"
        };
    }

    return {
        true,
        std::move(result),
        {}
    };
}

}  // namespace cad2sim::mesh
