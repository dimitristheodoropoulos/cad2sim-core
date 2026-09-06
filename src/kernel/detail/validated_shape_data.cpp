#include "validated_shape_data.hpp"

namespace cad2sim::kernel::detail {

ValidatedShapeData::ValidatedShapeData(
    const TopoDS_Shape& shape
)
    : shape_(shape) {}

const TopoDS_Shape& ValidatedShapeData::shape() const noexcept {
    return shape_;
}

}  // namespace cad2sim::kernel::detail
