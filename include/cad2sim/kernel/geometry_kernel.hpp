#pragma once

#include <cstddef>
#include <string>

namespace cad2sim::kernel {

struct ImportResult {
    bool success;
    bool shape_valid;
    std::string error;
};

struct TopologyCounts {
    std::size_t solids;
    std::size_t shells;
    std::size_t faces;
    std::size_t wires;
    std::size_t edges;
    std::size_t vertices;
};

class GeometryKernel {
public:
    ImportResult import_step(const std::string& path) const;
    TopologyCounts inspect_topology(const std::string& path) const;
};

}  // namespace cad2sim::kernel
