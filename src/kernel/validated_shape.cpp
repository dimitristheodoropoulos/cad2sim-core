#include "cad2sim/kernel/validated_shape.hpp"

#include <utility>

#include "detail/validated_shape_data.hpp"

namespace cad2sim::kernel {

ValidatedShape::ValidatedShape(
    std::shared_ptr<const detail::ValidatedShapeData> data
)
    : data_(std::move(data)) {}

bool ValidatedShape::valid() const noexcept {
    return static_cast<bool>(data_);
}

const TopoDS_Shape& ValidatedShape::shape() const noexcept {
    return data_->shape();
}

}  // namespace cad2sim::kernel
