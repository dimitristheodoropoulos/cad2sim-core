#pragma once

#include <TopoDS_Shape.hxx>

namespace cad2sim::kernel::detail {

class ValidatedShapeData {
public:
    explicit ValidatedShapeData(
        const TopoDS_Shape& shape
    );

    const TopoDS_Shape& shape() const noexcept;

private:
    TopoDS_Shape shape_;
};

}  // namespace cad2sim::kernel::detail
