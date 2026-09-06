#pragma once

#include <memory>

namespace cad2sim::mesh {
class SurfaceMesher;
}

namespace cad2sim::engineering {
class RegionIdentifier;
}

namespace cad2sim::kernel {

namespace detail {
class ValidatedShapeData;
}

class GeometryKernel;

class ValidatedShape {
public:
    ValidatedShape() = default;

    bool valid() const noexcept;

private:
    explicit ValidatedShape(
        std::shared_ptr<const detail::ValidatedShapeData> data
    );

    std::shared_ptr<const detail::ValidatedShapeData> data_;

    friend class cad2sim::mesh::SurfaceMesher;
    friend class cad2sim::engineering::RegionIdentifier;
    friend class GeometryKernel;
};

}  // namespace cad2sim::kernel
