#pragma once

#include <string>

namespace cad2sim::kernel {

struct ImportResult {
    bool success;
    bool shape_valid;
    std::string error;
};

class GeometryKernel {
public:
    ImportResult import_step(const std::string& path) const;
};

}  // namespace cad2sim::kernel
