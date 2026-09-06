#pragma once

#include <string>

#include <TopoDS_Shape.hxx>

namespace cad2sim::kernel::detail {

struct ShapeLoadResult {
    bool success;
    TopoDS_Shape shape;
    std::string diagnostic;
};

ShapeLoadResult load_step_shape(
    const std::string& path
);

}  // namespace cad2sim::kernel::detail
