# CAD2Sim-Core

C++ CAD-to-simulation preprocessing framework for 3D computational geometry, B-Rep analysis, finite-element meshing, and engineering model generation.

## Purpose

CAD2Sim-Core is an engineering software project focused on the preprocessing stage between 3D CAD geometry and simulation-ready engineering models.

The project explores a maintainable C++ architecture for:

- importing and inspecting 3D CAD geometry
- working with B-Rep topology
- applying 3D mathematical and computational-geometry operations
- recognizing geometric features
- preparing geometry for finite-element meshing
- evaluating mesh quality
- generating structured engineering-model data for downstream analysis

The project is intentionally focused on the computational and preprocessing layer rather than on a full CAD editor or complete physics solver.

## Target Engineering Workflow

```text
3D CAD
  │
  ▼
CAD Import
  │
  ▼
Geometry Kernel / B-Rep
  │
  ▼
Geometry Analysis
  │
  ├── Topology inspection
  ├── Geometric properties
  ├── Feature recognition
  └── Validity checks
  │
  ▼
Mesh Preprocessing
  │
  ├── Surface/volume meshing
  ├── Mesh quality evaluation
  └── Refinement preparation
  │
  ▼
Engineering Model
  │
  ├── Regions
  ├── Materials
  ├── Boundary-condition-ready entities
  └── Simulation metadata
```

## Technical Focus

* C++ engineering software development
* 3D mathematics
* computational geometry
* B-Rep topology
* CAD kernel integration
* finite-element preprocessing
* mesh generation and quality evaluation
* deterministic engineering data models
* automated testing and verification
* performance and robustness

## Geometry Kernel Strategy

The initial implementation is intended to use an open CAD kernel for development and verification.

The architecture will isolate kernel-specific functionality behind a geometry-kernel abstraction so that the application layer is not tightly coupled to a single implementation.

This separation is intended to provide a technically credible path toward supporting alternative commercial geometry kernels in the future.

The project does not claim prior professional experience with ACIS or Parasolid.

## Engineering Philosophy

CAD2Sim-Core follows an incremental engineering workflow:

1. define requirements
2. establish architecture
3. implement small testable components
4. verify behavior with automated tests
5. measure robustness and performance
6. document traceability between requirements and implementation

The repository is intended to demonstrate engineering reasoning and software quality rather than merely provide a collection of CAD demonstrations.

## Repository Structure

```text
cad2sim-core/
├── CMakeLists.txt
├── README.md
├── LICENSE
├── .gitignore
├── docs/
│   ├── requirements.md
│   ├── architecture.md
│   ├── geometry_model.md
│   ├── meshing.md
│   └── verification.md
├── include/
├── src/
├── tests/
├── examples/
├── benchmarks/
└── tools/
```

## Build and Verification

### Dependencies

The core build requires:

* CMake 3.20 or newer
* a C++17-compatible compiler
* OpenCASCADE Technology (OCCT)

OpenCASCADE is discovered by CMake using:

```cmake
find_package(OpenCASCADE REQUIRED)
```

### Configure and Build

From the repository root:

```bash
cmake -S . -B build
cmake --build build --parallel
```

### Run the Test Suite

The complete CTest suite can be executed with:

```bash
ctest --test-dir build --output-on-failure
```

The current verification baseline contains 18 registered CTest tests and is expected to report:

```text
100% tests passed, 0 tests failed out of 18
```

For a clean verification, use a separate build directory:

```bash
cmake -S . -B build-clean
cmake --build build-clean --parallel
ctest --test-dir build-clean --output-on-failure
```

## Requirements Traceability

The project requirements are maintained in:

`docs/requirements.md`

The requirements are derived from the technical scope of a Senior Software Engineer – 3D Modeling role and are mapped to concrete project evidence without overstating professional experience.

## Verification

Verification will include:

* unit testing
* integration testing
* geometry-kernel validation
* topology consistency checks
* mesh-quality checks
* regression testing
* deterministic output checks
* performance measurements where appropriate

Detailed verification criteria will be maintained in `docs/verification.md`.

## Status

The current repository baseline includes implemented and verified core geometry, topology, preprocessing, mesh-quality, engineering-model, deterministic-regression, and robust-error-handling functionality.

The current CTest baseline contains 18 registered tests, with 18/18 passing and 0 failures.
