cmake_minimum_required(VERSION 3.20)

project(CAD2SimCore
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(OpenCASCADE REQUIRED
    COMPONENTS
        FoundationClasses
        ModelingData
        ModelingAlgorithms
        Visualization
        ApplicationFramework
        DataExchange
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cad2sim_core
    src/core.cpp
    src/vec3.cpp
    src/transform3.cpp
    src/kernel/geometry_kernel.cpp
)

target_include_directories(cad2sim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${OpenCASCADE_INCLUDE_DIR}
)

target_link_libraries(cad2sim_core
    PUBLIC
        TKernel
        TKMath
        TKG2d
        TKG3d
        TKGeomBase
        TKBRep
        TKGeomAlgo
        TKTopAlgo
        TKPrim
        TKBO
        TKShHealing
        TKBool
        TKSTEPBase
        TKSTEPAttr
        TKSTEP209
        TKSTEP
)

add_executable(cad2sim_core_unit
    tests/unit/test_core.cpp
)

target_link_libraries(cad2sim_core_unit
    PRIVATE
        cad2sim_core
)

enable_testing()

add_test(
    NAME cad2sim_core_unit
    COMMAND cad2sim_core_unit
)

add_executable(cad2sim_vec3_unit
    tests/unit/math/test_vec3.cpp
)

target_link_libraries(cad2sim_vec3_unit
    PRIVATE
        cad2sim_core
)

add_test(
    NAME cad2sim_vec3_unit
    COMMAND cad2sim_vec3_unit
)

add_executable(cad2sim_transform3_unit
    tests/unit/math/test_transform3.cpp
)

target_link_libraries(cad2sim_transform3_unit
    PRIVATE
        cad2sim_core
)

add_test(
    NAME cad2sim_transform3_unit
    COMMAND cad2sim_transform3_unit
)

add_executable(cad2sim_primitives_unit
    tests/unit/geometry/test_primitives.cpp
)

target_link_libraries(cad2sim_primitives_unit
    PRIVATE
        cad2sim_core
)

add_test(
    NAME cad2sim_primitives_unit
    COMMAND cad2sim_primitives_unit
)

add_executable(cad2sim_advanced_primitives_unit
    tests/unit/geometry/test_advanced_primitives.cpp
)

target_link_libraries(cad2sim_advanced_primitives_unit
    PRIVATE
        cad2sim_core
)

add_test(
    NAME cad2sim_advanced_primitives_unit
    COMMAND cad2sim_advanced_primitives_unit
)

add_executable(cad2sim_step_import_integration
    tests/integration/kernel/test_step_import.cpp
)

target_link_libraries(cad2sim_step_import_integration
    PRIVATE
        cad2sim_core
)

target_compile_definitions(cad2sim_step_import_integration
    PRIVATE
        CAD2SIM_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME cad2sim_step_import_integration
    COMMAND cad2sim_step_import_integration
)

add_executable(cad2sim_brep_topology_integration
    tests/integration/topology/test_brep_topology.cpp
)

target_link_libraries(cad2sim_brep_topology_integration
    PRIVATE
        cad2sim_core
)

target_compile_definitions(cad2sim_brep_topology_integration
    PRIVATE
        CAD2SIM_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

add_test(
    NAME cad2sim_brep_topology_integration
    COMMAND cad2sim_brep_topology_integration
)
