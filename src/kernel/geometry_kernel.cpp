#include "cad2sim/kernel/geometry_kernel.hpp"


#include <filesystem>
#include <memory>
#include <utility>

#include "geometry_validation.hpp"
#include "detail/step_shape_loader.hpp"
#include "detail/validated_shape_data.hpp"


#include <Bnd_Box.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepGProp.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRepTools.hxx>
#include <GProp_GProps.hxx>
#include <STEPControl_Reader.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <gp_Ax1.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>

#include <GeomAbs_CurveType.hxx>
#include <GeomAbs_SurfaceType.hxx>

namespace cad2sim::kernel {

ImportResult GeometryKernel::import_step(const std::string& path) const {
    const auto loaded =
        detail::load_step_shape(path);

    if (!loaded.success) {
        return {
            false,
            false,
            loaded.diagnostic
        };
    }

    const auto validation =
        detail::validate_shape(loaded.shape);

    if (!validation.valid) {
        return {
            false,
            false,
            validation.diagnostic
        };
    }

    return {
        true,
        true,
        {}
    };
}

ValidatedShapeResult GeometryKernel::load_validated_shape(
    const std::string& path
) const {
    const auto loaded =
        detail::load_step_shape(path);

    if (!loaded.success) {
        return {
            false,
            {},
            loaded.diagnostic
        };
    }

    const auto validation =
        detail::validate_shape(loaded.shape);

    if (!validation.valid) {
        return {
            false,
            {},
            validation.diagnostic
        };
    }

    auto data =
        std::make_shared<const detail::ValidatedShapeData>(
            loaded.shape
        );

    return {
        true,
        ValidatedShape(std::move(data)),
        {}
    };
}


TopologyCounts GeometryKernel::inspect_topology(
    const std::string& path
) const {
    if (!std::filesystem::exists(path)) {
        return {0, 0, 0, 0, 0, 0};
    }

    STEPControl_Reader reader;

    const IFSelect_ReturnStatus status =
        reader.ReadFile(path.c_str());

    if (status != IFSelect_RetDone) {
        return {0, 0, 0, 0, 0, 0};
    }

    const Standard_Integer transferred =
        reader.TransferRoots();

    if (transferred <= 0) {
        return {0, 0, 0, 0, 0, 0};
    }

    const TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        return {0, 0, 0, 0, 0, 0};
    }

    TopologyCounts counts{};

    for (TopExp_Explorer explorer(shape, TopAbs_SOLID);
         explorer.More();
         explorer.Next()) {
        ++counts.solids;
    }

    for (TopExp_Explorer explorer(shape, TopAbs_SHELL);
         explorer.More();
         explorer.Next()) {
        ++counts.shells;
    }

    for (TopExp_Explorer explorer(shape, TopAbs_FACE);
         explorer.More();
         explorer.Next()) {
        ++counts.faces;
    }

    for (TopExp_Explorer explorer(shape, TopAbs_WIRE);
         explorer.More();
         explorer.Next()) {
        ++counts.wires;
    }

    for (TopExp_Explorer explorer(shape, TopAbs_EDGE);
         explorer.More();
         explorer.Next()) {
        ++counts.edges;
    }

    for (TopExp_Explorer explorer(shape, TopAbs_VERTEX);
         explorer.More();
         explorer.Next()) {
        ++counts.vertices;
    }

    return counts;
}

}  // namespace cad2sim::kernel


namespace cad2sim::kernel::detail {

ValidationResult validate_shape(const TopoDS_Shape& shape) {
    if (shape.IsNull()) {
        return {
            false,
            "Geometry validation failed: shape is null"
        };
    }

    const BRepCheck_Analyzer analyzer(shape);

    if (!analyzer.IsValid()) {
        return {
            false,
            "Geometry validation failed: B-Rep shape is invalid"
        };
    }

    return {
        true,
        {}
    };
}

}  // namespace cad2sim::kernel::detail

namespace cad2sim::kernel {

ValidationResult GeometryKernel::validate_step(
    const std::string& path
) const {
    if (!std::filesystem::exists(path)) {
        return {
            false,
            "Geometry validation failed: STEP file does not exist: " +
            path
        };
    }

    STEPControl_Reader reader;

    const IFSelect_ReturnStatus status =
        reader.ReadFile(path.c_str());

    if (status != IFSelect_RetDone) {
        return {
            false,
            "Geometry validation failed: failed to read STEP file: " +
            path
        };
    }

    const Standard_Integer transferred =
        reader.TransferRoots();

    if (transferred <= 0) {
        return {
            false,
            "Geometry validation failed: STEP file contains no transferable roots: " +
            path
        };
    }

    const TopoDS_Shape shape = reader.OneShape();

    return detail::validate_shape(shape);
}

GeometryProperties GeometryKernel::inspect_geometry_properties(
    const std::string& path
) const {
    GeometryProperties properties{};

    if (!std::filesystem::exists(path)) {
        return properties;
    }

    STEPControl_Reader reader;

    const IFSelect_ReturnStatus status =
        reader.ReadFile(path.c_str());

    if (status != IFSelect_RetDone) {
        return properties;
    }

    const Standard_Integer transferred =
        reader.TransferRoots();

    if (transferred <= 0) {
        return properties;
    }

    const TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        return properties;
    }

    // Bounding box.
    Bnd_Box box;
    BRepBndLib::Add(shape, box);

    if (!box.IsVoid()) {
        Standard_Real min_x;
        Standard_Real min_y;
        Standard_Real min_z;
        Standard_Real max_x;
        Standard_Real max_y;
        Standard_Real max_z;

        box.Get(
            min_x,
            min_y,
            min_z,
            max_x,
            max_y,
            max_z
        );

        properties.bounding_box = {
            min_x,
            min_y,
            min_z,
            max_x,
            max_y,
            max_z
        };
    }

    // Volume and centroid.
    GProp_GProps volume_properties;
    BRepGProp::VolumeProperties(
        shape,
        volume_properties
    );

    properties.volume = volume_properties.Mass();

    const gp_Pnt volume_centroid =
        volume_properties.CentreOfMass();

    properties.centroid = {
        volume_centroid.X(),
        volume_centroid.Y(),
        volume_centroid.Z()
    };

    // Unique faces.
    TopTools_IndexedMapOfShape face_map;

    TopExp::MapShapes(
        shape,
        TopAbs_FACE,
        face_map
    );

    properties.faces.reserve(
        static_cast<std::size_t>(face_map.Extent())
    );

    for (Standard_Integer index = 1;
         index <= face_map.Extent();
         ++index) {
        const TopoDS_Face face =
            TopoDS::Face(face_map(index));

        GProp_GProps surface_properties;

        BRepGProp::SurfaceProperties(
            face,
            surface_properties
        );

        const gp_Pnt face_centroid =
            surface_properties.CentreOfMass();

        FaceProperties face_properties{};

        face_properties.index =
            static_cast<std::size_t>(index - 1);

        face_properties.area =
            surface_properties.Mass();

        face_properties.centroid = {
            face_centroid.X(),
            face_centroid.Y(),
            face_centroid.Z()
        };

        // Compute a representative surface normal.
        Standard_Real u_min;
        Standard_Real u_max;
        Standard_Real v_min;
        Standard_Real v_max;

        BRepTools::UVBounds(
            face,
            u_min,
            u_max,
            v_min,
            v_max
        );

        const Standard_Real u =
            (u_min + u_max) / 2.0;

        const Standard_Real v =
            (v_min + v_max) / 2.0;

        BRepAdaptor_Surface surface(
            face,
            Standard_True
        );

        BRepLProp_SLProps local_properties(
            surface,
            u,
            v,
            1,
            1.0e-7
        );

        if (local_properties.IsNormalDefined()) {
            const gp_Dir normal =
                local_properties.Normal();

            double nx = normal.X();
            double ny = normal.Y();
            double nz = normal.Z();

            if (face.Orientation() == TopAbs_REVERSED) {
                nx = -nx;
                ny = -ny;
                nz = -nz;
            }

            face_properties.normal = {
                nx,
                ny,
                nz
            };
        }

        properties.faces.push_back(face_properties);
    }

    // Unique edges.
    TopTools_IndexedMapOfShape edge_map;

    TopExp::MapShapes(
        shape,
        TopAbs_EDGE,
        edge_map
    );

    properties.edges.reserve(
        static_cast<std::size_t>(edge_map.Extent())
    );

    for (Standard_Integer index = 1;
         index <= edge_map.Extent();
         ++index) {
        const TopoDS_Edge edge =
            TopoDS::Edge(edge_map(index));

        GProp_GProps edge_properties;

        BRepGProp::LinearProperties(
            edge,
            edge_properties
        );

        properties.edges.push_back({
            static_cast<std::size_t>(index - 1),
            edge_properties.Mass()
        });
    }

    return properties;
}

std::vector<FaceDescriptor> GeometryKernel::inspect_faces(
    const std::string& path
) const {
    std::vector<FaceDescriptor> descriptors;

    if (!std::filesystem::exists(path)) {
        return descriptors;
    }

    STEPControl_Reader reader;
    if (reader.ReadFile(path.c_str()) != IFSelect_RetDone) {
        return descriptors;
    }

    if (reader.TransferRoots() <= 0) {
        return descriptors;
    }

    const TopoDS_Shape shape = reader.OneShape();
    if (shape.IsNull()) {
        return descriptors;
    }

    // Get unique faces using indexed map
    TopTools_IndexedMapOfShape face_map;
    TopExp::MapShapes(shape, TopAbs_FACE, face_map);

    descriptors.reserve(static_cast<std::size_t>(face_map.Extent()));

    for (Standard_Integer i = 1; i <= face_map.Extent(); ++i) {
        const TopoDS_Face face = TopoDS::Face(face_map(i));

        // Compute area and skip degenerate faces
        GProp_GProps surface_props;
        BRepGProp::SurfaceProperties(face, surface_props);
        const double area = surface_props.Mass();

        if (area < 1e-12) {
            continue;
        }

        // Classify surface type
        BRepAdaptor_Surface adaptor(face, Standard_True);
        SurfaceType type = SurfaceType::Unknown;

        switch (adaptor.GetType()) {
            case GeomAbs_Plane:
                type = SurfaceType::Plane;
                break;
            case GeomAbs_Cylinder:
                type = SurfaceType::Cylinder;
                break;
            default:
                break;
        }

        const gp_Pnt centroid = surface_props.CentreOfMass();

        descriptors.push_back({
            descriptors.size(),
            type,
            area,
            {centroid.X(), centroid.Y(), centroid.Z()}
        });
    }

    return descriptors;
}


namespace {

CurveType classify_curve(const TopoDS_Edge& edge)
{
    BRepAdaptor_Curve adaptor(edge);

    switch (adaptor.GetType()) {
        case GeomAbs_Line:
            return CurveType::Line;
        case GeomAbs_Circle:
            return CurveType::Circle;
        default:
            return CurveType::Other;
    }
}

SurfaceType classify_surface(const TopoDS_Face& face)
{
    BRepAdaptor_Surface adaptor(face, Standard_True);

    switch (adaptor.GetType()) {
        case GeomAbs_Plane:
            return SurfaceType::Plane;
        case GeomAbs_Cylinder:
            return SurfaceType::Cylinder;
        default:
            return SurfaceType::Unknown;
    }
}

}  // namespace

FaceTopologyResult GeometryKernel::inspect_face_topology(
    const std::string& path
) const {
    FaceTopologyResult result{};

    if (!std::filesystem::exists(path)) {
        return result;
    }

    STEPControl_Reader reader;

    if (reader.ReadFile(path.c_str()) != IFSelect_RetDone) {
        return result;
    }

    if (reader.TransferRoots() <= 0) {
        return result;
    }

    const TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        return result;
    }

    TopTools_IndexedMapOfShape face_map;
    TopTools_IndexedMapOfShape edge_map;

    TopExp::MapShapes(
        shape,
        TopAbs_FACE,
        face_map
    );

    TopExp::MapShapes(
        shape,
        TopAbs_EDGE,
        edge_map
    );

    TopTools_IndexedDataMapOfShapeListOfShape edge_faces;

    TopExp::MapShapesAndAncestors(
        shape,
        TopAbs_EDGE,
        TopAbs_FACE,
        edge_faces
    );

    result.faces.reserve(
        static_cast<std::size_t>(face_map.Extent())
    );

    for (Standard_Integer i = 1;
         i <= face_map.Extent();
         ++i) {
        const TopoDS_Face face =
            TopoDS::Face(face_map(i));

        GProp_GProps surface_properties;

        BRepGProp::SurfaceProperties(
            face,
            surface_properties
        );

        const double area =
            surface_properties.Mass();

        if (area < 1.0e-12) {
            continue;
        }

        const gp_Pnt centroid =
            surface_properties.CentreOfMass();

        FaceTopologyDescriptor descriptor{};

        descriptor.index =
            static_cast<std::size_t>(i - 1);

        descriptor.surface_type =
            classify_surface(face);

        descriptor.area = area;

        descriptor.centroid = {
            centroid.X(),
            centroid.Y(),
            centroid.Z()
        };

        descriptor.orientation =
            face.Orientation() == TopAbs_REVERSED
                ? FaceOrientation::Reversed
                : FaceOrientation::Forward;

        BRepAdaptor_Surface adaptor(
            face,
            Standard_True
        );

        if (adaptor.GetType() == GeomAbs_Plane) {
            const gp_Pln plane =
                adaptor.Plane();

            const gp_Dir normal =
                plane.Axis().Direction();

            descriptor.normal = {
                normal.X(),
                normal.Y(),
                normal.Z()
            };
        }

        if (adaptor.GetType() == GeomAbs_Cylinder) {
            const gp_Cylinder cylinder =
                adaptor.Cylinder();

            const gp_Ax1 axis =
                cylinder.Axis();

            const gp_Pnt origin =
                axis.Location();

            const gp_Dir direction =
                axis.Direction();

            descriptor.radius =
                cylinder.Radius();

            descriptor.axis_origin = {
                origin.X(),
                origin.Y(),
                origin.Z()
            };

            descriptor.axis_direction = {
                direction.X(),
                direction.Y(),
                direction.Z()
            };
        }

        for (TopExp_Explorer edge_exp(
                 face,
                 TopAbs_EDGE
             );
             edge_exp.More();
             edge_exp.Next()) {
            const TopoDS_Edge edge =
                TopoDS::Edge(edge_exp.Current());

            const Standard_Integer edge_index =
                edge_map.FindIndex(edge);

            if (edge_index > 0) {
                descriptor.edge_indices.push_back(
                    static_cast<std::size_t>(
                        edge_index - 1
                    )
                );
            }
        }

        result.faces.push_back(
            std::move(descriptor)
        );
    }

    result.edges.reserve(
        static_cast<std::size_t>(edge_map.Extent())
    );

    for (Standard_Integer i = 1;
         i <= edge_map.Extent();
         ++i) {
        const TopoDS_Edge edge =
            TopoDS::Edge(edge_map(i));

        GProp_GProps edge_properties;

        BRepGProp::LinearProperties(
            edge,
            edge_properties
        );

        EdgeTopologyDescriptor descriptor{};

        descriptor.index =
            static_cast<std::size_t>(i - 1);

        descriptor.curve_type =
            classify_curve(edge);

        descriptor.length =
            edge_properties.Mass();

        const Standard_Integer ancestor_index =
            edge_faces.FindIndex(edge);

        if (ancestor_index > 0) {
            const auto& ancestors =
                edge_faces.FindFromIndex(ancestor_index);

            descriptor.adjacent_face_indices.reserve(
                static_cast<std::size_t>(
                    ancestors.Extent()
                )
            );

            for (auto it = ancestors.cbegin();
                 it != ancestors.cend();
                 ++it) {
                const Standard_Integer face_index =
                    face_map.FindIndex(*it);

                if (face_index > 0) {
                    descriptor.adjacent_face_indices.push_back(
                        static_cast<std::size_t>(
                            face_index - 1
                        )
                    );
                }
            }
        }

        result.edges.push_back(
            std::move(descriptor)
        );
    }

    return result;
}

}  // namespace cad2sim::kernel
