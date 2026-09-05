#include "cad2sim/kernel/geometry_kernel.hpp"

#include <filesystem>

#include <BRepCheck_Analyzer.hxx>
#include <STEPControl_Reader.hxx>
#include <TopoDS_Shape.hxx>

namespace cad2sim::kernel {

ImportResult GeometryKernel::import_step(const std::string& path) const {
    if (!std::filesystem::exists(path)) {
        return {
            false,
            false,
            "STEP file does not exist: " + path
        };
    }

    STEPControl_Reader reader;

    const IFSelect_ReturnStatus status = reader.ReadFile(path.c_str());

    if (status != IFSelect_RetDone) {
        return {
            false,
            false,
            "Failed to read STEP file: " + path
        };
    }

    const Standard_Integer transferred = reader.TransferRoots();

    if (transferred <= 0) {
        return {
            false,
            false,
            "STEP file contains no transferable roots: " + path
        };
    }

    const TopoDS_Shape shape = reader.OneShape();

    if (shape.IsNull()) {
        return {
            false,
            false,
            "STEP import produced a null shape: " + path
        };
    }

    const BRepCheck_Analyzer analyzer(shape);
    const bool valid = analyzer.IsValid();

    if (!valid) {
        return {
            false,
            false,
            "Imported STEP shape is geometrically invalid: " + path
        };
    }

    return {
        true,
        true,
        {}
    };
}

}  // namespace cad2sim::kernel
