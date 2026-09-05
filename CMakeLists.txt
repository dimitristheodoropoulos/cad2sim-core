cmake_minimum_required(VERSION 3.20)

project(CAD2SimCore
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(cad2sim_core
    src/core.cpp
    src/vec3.cpp
    src/transform3.cpp
)

target_include_directories(cad2sim_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
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
