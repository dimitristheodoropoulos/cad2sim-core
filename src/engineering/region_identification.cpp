#include "cad2sim/engineering/region_identification.hpp"

#include <utility>

#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>

#include "../kernel/detail/validated_shape_data.hpp"

namespace cad2sim::engineering {

RegionIdentificationResult RegionIdentifier::identify(
    const kernel::ValidatedShape& shape
) const {
    if (!shape.valid()) {
        return {
            false,
            {},
            "Engineering region identification failed: validated shape is invalid"
        };
    }

    const TopoDS_Shape& occt_shape =
        shape.data_->shape();

    std::vector<EngineeringRegion> regions;

    std::size_t face_index = 0;

    for (TopExp_Explorer explorer(
             occt_shape,
             TopAbs_FACE
         );
         explorer.More();
         explorer.Next(), ++face_index) {

        const TopoDS_Face face =
            TopoDS::Face(explorer.Current());

        if (face.IsNull()) {
            continue;
        }

        const std::size_t region_index =
            regions.size();

        regions.push_back({
            region_index,
            {
                RegionEntityType::Face,
                face_index
            }
        });
    }

    if (regions.empty()) {
        return {
            false,
            {},
            "Engineering region identification failed: shape contains no faces"
        };
    }

    return {
        true,
        std::move(regions),
        {}
    };
}

}  // namespace cad2sim::engineering
