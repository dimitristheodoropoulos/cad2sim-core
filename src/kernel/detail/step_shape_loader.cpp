#include "step_shape_loader.hpp"

#include <filesystem>

#include <IFSelect_ReturnStatus.hxx>
#include <STEPControl_Reader.hxx>

namespace cad2sim::kernel::detail {

ShapeLoadResult load_step_shape(
    const std::string& path
) {
    if (!std::filesystem::exists(path)) {
        return {
            false,
            {},
            "STEP file does not exist: " + path
        };
    }

    STEPControl_Reader reader;

    const IFSelect_ReturnStatus status =
        reader.ReadFile(path.c_str());

    if (status != IFSelect_RetDone) {
        return {
            false,
            {},
            "Failed to read STEP file: " + path
        };
    }

    const Standard_Integer transferred =
        reader.TransferRoots();

    if (transferred <= 0) {
        return {
            false,
            {},
            "STEP file contains no transferable roots: " + path
        };
    }

    const TopoDS_Shape shape =
        reader.OneShape();

    if (shape.IsNull()) {
        return {
            false,
            {},
            "STEP import produced a null shape: " + path
        };
    }

    return {
        true,
        shape,
        {}
    };
}

}  // namespace cad2sim::kernel::detail
