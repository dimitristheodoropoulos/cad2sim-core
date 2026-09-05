#include "cad2sim/features/feature_recognition.hpp"

#include <filesystem>

#include <BRepAdaptor_Surface.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <STEPControl_Reader.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include <GeomAbs_SurfaceType.hxx>

#include "cad2sim/kernel/geometry_kernel.hpp"

namespace cad2sim::features {

namespace {

constexpr double kMinimumFaceArea = 1.0e-12;

bool is_non_degenerate_face(const TopoDS_Face& face) {
    GProp_GProps properties;

    BRepGProp::SurfaceProperties(
        face,
        properties
    );

    return properties.Mass() > kMinimumFaceArea;
}

}  // namespace

FeatureRecognitionResult FeatureRecognizer::recognize(
    const std::string& path
) const {
    FeatureRecognitionResult result;

    if (!std::filesystem::exists(path)) {
        return result;
    }

    cad2sim::kernel::GeometryKernel kernel;

    const auto validation = kernel.validate_step(path);

    if (!validation.valid) {
        return result;
    }

    STEPControl_Reader reader;

    const IFSelect_ReturnStatus status =
        reader.ReadFile(path.c_str());

    if (status != IFSelect_RetDone) {
        return result;
    }

    const Standard_Integer transferred =
        reader.TransferRoots();

    if (transferred <= 0) {
        return result;
    }

    const TopoDS_Shape shape =
        reader.OneShape();

    if (shape.IsNull()) {
        return result;
    }

    std::size_t feature_index = 0;

    for (TopExp_Explorer explorer(shape, TopAbs_FACE);
         explorer.More();
         explorer.Next()) {
        const TopoDS_Face face =
            TopoDS::Face(explorer.Current());

        if (!is_non_degenerate_face(face)) {
            continue;
        }

        BRepAdaptor_Surface surface(
            face,
            Standard_True
        );

        switch (surface.GetType()) {
        case GeomAbs_Plane:
            result.features.push_back({
                FeatureType::PlanarFace,
                FeatureStatus::Confirmed,
                feature_index++,
                {}
            });
            break;

        case GeomAbs_Cylinder:
            result.features.push_back({
                FeatureType::CylindricalFace,
                FeatureStatus::Confirmed,
                feature_index++,
                {}
            });
            break;

        default:
            break;
        }
    }

    return result;
}

}  // namespace cad2sim::features
