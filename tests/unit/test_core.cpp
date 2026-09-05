#include <cassert>

#include "cad2sim/core.hpp"

int main() {
    assert(cad2sim::core_version() == 1);
    return 0;
}
