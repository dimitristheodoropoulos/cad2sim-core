#pragma once

#include "cad2sim/kernel/geometry_kernel.hpp"

class TopoDS_Shape;

namespace cad2sim::kernel::detail {

ValidationResult validate_shape(const TopoDS_Shape& shape);

}  // namespace cad2sim::kernel::detail
