# CAD2Sim-Core Requirements

## 1. Purpose

This document defines the initial technical requirements for CAD2Sim-Core.

The requirements establish the project's scope before implementation begins and provide traceability between the target engineering capabilities and concrete verification evidence.

The project is designed around the technical requirements of a 3D modeling / engineering-software role while making no unsupported claims about professional experience.

---

## 2. Requirement Identification

Requirements use the following identifiers:

* `CAD2SIM-REQ-001` through `CAD2SIM-REQ-020`
* `CAD2SIM-NFR-001` through `CAD2SIM-NFR-010`

`REQ` denotes functional or technical requirements.

`NFR` denotes non-functional requirements.

---

## 3. Core Technical Requirements

### CAD2SIM-REQ-001 — C++ Implementation

The core application shall be implemented in modern C++.

**Target evidence:**

* C++ source code
* CMake build configuration
* automated compilation

**Verification:**

* clean build
* compiler diagnostics
* automated test execution

---

### CAD2SIM-REQ-002 — 3D Vector Mathematics

The system shall provide tested mathematical primitives required for 3D geometry processing.

The initial scope shall include:

* vectors
* dot products
* cross products
* normalization
* distances
* projections
* coordinate transformations

**Verification:**

* deterministic unit tests
* numerical tolerance checks

---

### CAD2SIM-REQ-003 — Geometric Primitives

The system shall support computational operations on fundamental 3D geometric entities.

The initial scope shall include, where applicable:

* points
* lines
* planes
* curves
* surfaces
* coordinate frames

**Verification:**

* unit tests
* controlled geometry cases

---

### CAD2SIM-REQ-004 — CAD Geometry Import

The system shall support importing a standard 3D CAD representation through the selected geometry-kernel integration.

The initial supported exchange format shall be STEP.

**Verification:**

* import representative STEP models
* validate imported topology
* report import failures deterministically

---

### CAD2SIM-REQ-005 — B-Rep Representation

The system shall inspect boundary-representation topology.

The system shall be able to identify and traverse, where supported by the kernel:

* solids
* shells
* faces
* wires
* edges
* vertices

**Verification:**

* topology-count tests
* controlled reference models
* consistency checks

---

### CAD2SIM-REQ-006 — Geometry Validity Analysis

The system shall provide geometry-validity checks before downstream preprocessing.

Validity failures shall be reported without silently producing an invalid engineering model.

**Verification:**

* valid-model tests
* intentionally invalid or problematic geometry cases
* deterministic diagnostic output

---

### CAD2SIM-REQ-007 — Geometric Property Extraction

The system shall extract relevant geometric properties from CAD entities.

The initial scope shall include appropriate properties such as:

* bounding boxes
* surface normals
* areas
* lengths
* volumes where applicable
* centroids where applicable

**Verification:**

* analytical reference cases
* tolerance-based numerical comparison

---

### CAD2SIM-REQ-008 — Geometric Feature Recognition

The system shall identify selected geometric features relevant to engineering preprocessing.

The initial feature set shall include representative features such as:

* planar faces
* cylindrical faces
* holes
* pockets
* fillets
* chamfers

Feature recognition shall be implemented incrementally and shall report unsupported or ambiguous cases explicitly.

**Verification:**

* controlled CAD models
* feature classification tests
* false-positive/false-negative regression cases

---

### CAD2SIM-REQ-009 — Geometry Transformations

The system shall support transformations required for engineering geometry processing.

The initial scope shall include:

* translation
* rotation
* scaling where appropriate
* transformation between coordinate frames

**Verification:**

* deterministic transformation tests
* round-trip tests where applicable

---

### CAD2SIM-REQ-010 — Geometry Kernel Abstraction

Kernel-specific functionality shall be isolated behind an application-level abstraction.

The application layer shall not depend unnecessarily on implementation-specific kernel details.

The architecture shall permit an alternative kernel adapter to be introduced without redesigning the complete preprocessing pipeline.

**Note:**
This architectural requirement does not constitute prior professional experience with ACIS or Parasolid.

**Verification:**

* interface-level tests
* dependency-boundary review
* architecture inspection

---

### CAD2SIM-REQ-011 — Finite-Element Mesh Preparation

The system shall prepare valid CAD geometry for finite-element meshing.

The preprocessing stage shall support the generation of surface and/or volume mesh data through the selected meshing technology.

**Verification:**

* representative geometry models
* successful mesh generation
* failure handling

---

### CAD2SIM-REQ-012 — Mesh Quality Evaluation

The system shall evaluate generated meshes using appropriate quality indicators.

The initial implementation shall support metrics appropriate to the selected element types, such as:

* element count
* aspect ratio
* minimum/maximum angle where applicable
* skewness where applicable
* Jacobian-related quality indicators where applicable

**Verification:**

* known-good mesh cases
* deliberately degraded mesh cases
* deterministic metric calculations

---

### CAD2SIM-REQ-013 — Engineering Region Identification

The system shall provide a structured representation of engineering-relevant regions derived from CAD geometry.

Regions shall retain stable references to their originating geometric entities where possible.

**Verification:**

* controlled CAD models
* stable region identification
* regression tests

---

### CAD2SIM-REQ-014 — Simulation-Ready Metadata

The system shall produce structured engineering metadata suitable for downstream simulation preprocessing.

The model shall be capable of representing concepts such as:

* geometric regions
* material assignments
* analysis entities
* boundary-condition-ready references
* preprocessing metadata

The project shall not claim to implement a complete physics solver.

**Verification:**

* schema validation
* deterministic serialization
* round-trip tests where applicable

---

### CAD2SIM-REQ-015 — Deterministic Processing

For identical input geometry and identical processing parameters, the preprocessing pipeline shall produce deterministic results wherever deterministic behavior is technically applicable.

**Verification:**

* repeated-run comparisons
* serialized-output comparison
* regression tests

---

### CAD2SIM-REQ-016 — Robust Error Handling

The system shall detect and report recoverable processing failures explicitly.

Examples include:

* unsupported geometry
* invalid CAD input
* failed topology extraction
* mesh-generation failure
* invalid preprocessing parameters

The system shall avoid silent failure.

**Verification:**

* negative tests
* malformed-input tests
* error-reporting assertions

---

### CAD2SIM-REQ-017 — Automated Testing

Core geometry, topology, preprocessing, and mesh-quality functionality shall be covered by automated tests.

**Verification:**

* reproducible test execution
* regression suite
* documented test results

---

### CAD2SIM-REQ-018 — Engineering Regression Suite

The project shall maintain representative CAD and preprocessing cases for regression testing.

Regression cases shall cover:

* simple primitives
* topology variations
* recognized features
* invalid/problematic geometry
* meshing cases
* engineering metadata generation

**Verification:**

* automated regression execution
* documented baseline results

---

### CAD2SIM-REQ-019 — Performance Measurement

The project shall provide a mechanism for measuring preprocessing performance on representative models.

Measurements may include:

* input model complexity
* topology size
* mesh size
* processing time
* peak memory usage where measurable

**Verification:**

* repeatable benchmark runs
* documented benchmark methodology

---

### CAD2SIM-REQ-020 — Engineering Documentation

The project shall document:

* requirements
* architecture
* geometry model
* meshing strategy
* verification methodology
* known limitations

Documentation shall distinguish implemented capabilities from planned capabilities.

**Verification:**

* documentation review
* requirements-to-evidence traceability

---

## 4. Non-Functional Requirements

### CAD2SIM-NFR-001 — Maintainability

The codebase shall use modular interfaces and clearly separated responsibilities.

---

### CAD2SIM-NFR-002 — Testability

Core components shall be designed so that behavior can be tested independently where practical.

---

### CAD2SIM-NFR-003 — Portability

The build system shall support the development environment and avoid unnecessary platform-specific assumptions.

---

### CAD2SIM-NFR-004 — Reproducibility

Builds and tests shall use documented dependencies and repeatable commands.

---

### CAD2SIM-NFR-005 — Diagnostic Quality

Failures shall provide actionable diagnostic information where technically possible.

---

### CAD2SIM-NFR-006 — Separation of Concerns

CAD-kernel integration, geometry processing, meshing, engineering-model generation, and application orchestration shall remain separable architectural concerns.

---

### CAD2SIM-NFR-007 — Explicit Scope

The project shall not represent prototype functionality as production-certified engineering software.

---

### CAD2SIM-NFR-008 — Honest Experience Representation

Project documentation and CV-facing material shall distinguish:

* demonstrated project capability
* academic knowledge
* professional experience
* planned/future integrations

In particular, the project shall not claim professional ACIS or Parasolid experience unless such experience is actually acquired.

---

### CAD2SIM-NFR-009 — Verification Before Claim

A capability shall not be described as verified or production-ready until corresponding verification evidence exists.

The project may distinguish internally between implemented functionality and verified functionality, but externally documented capability claims shall be supported by corresponding verification evidence.

---

### CAD2SIM-NFR-010 — Traceability

Every implemented requirement shall eventually map to implementation evidence and verification evidence.

---

## 5. Target Role Mapping

The following mapping defines how the project addresses the major technical themes of the target 3D Modeling engineering role.

| Target capability                   | CAD2Sim-Core evidence                                           |
| ----------------------------------- | --------------------------------------------------------------- |
| C++                                 | `CAD2SIM-REQ-001`                                               |
| 3D mathematics                      | `CAD2SIM-REQ-002`, `CAD2SIM-REQ-003`                            |
| Computational geometry              | `CAD2SIM-REQ-002`–`CAD2SIM-REQ-009`                             |
| CAD/B-Rep modeling                  | `CAD2SIM-REQ-004`–`CAD2SIM-REQ-010`                             |
| Geometry kernel concepts            | `CAD2SIM-REQ-010`                                               |
| Finite-element preprocessing        | `CAD2SIM-REQ-011`, `CAD2SIM-REQ-012`                            |
| Engineering preprocessing           | `CAD2SIM-REQ-013`, `CAD2SIM-REQ-014`                            |
| Robust software                     | `CAD2SIM-REQ-016`                                               |
| Testing                             | `CAD2SIM-REQ-017`, `CAD2SIM-REQ-018`                            |
| Performance analysis                | `CAD2SIM-REQ-019`                                               |
| Maintainable architecture           | `CAD2SIM-NFR-001`, `CAD2SIM-NFR-006`                            |
| Analytical/problem-solving approach | Overall requirements, architecture and verification methodology |
| ACIS/Parasolid                      | Not claimed; architecture provides a future adapter boundary    |

---

## 6. Explicitly Out of Scope

The initial project does not claim to implement:

* a complete commercial CAD system
* a complete finite-element solver
* CFD, structural, or electromagnetic physics solvers
* production-certified engineering software
* physical hardware validation
* professional ACIS experience
* professional Parasolid experience
* production CAE software certification
* proprietary Gamma Technologies software or internal APIs

These boundaries are intentional and are part of the project's credibility.

---

## 7. Requirement Status Convention

Each requirement will eventually receive one of the following statuses:

* `PENDING` — defined but not yet implemented or verified
* `IMPLEMENTED` — implementation exists but verification is incomplete
* `VERIFIED` — implementation and verification evidence exist
* `OUT-OF-SCOPE` — explicitly excluded from the project

No requirement shall be marked `VERIFIED` without corresponding evidence.

---

## 8. Requirements-to-Evidence Matrix

This matrix defines the traceability structure that will connect each requirement to its implementation and verification evidence.

No source file, test, or verification result shall be listed until it actually exists in the repository.

| Requirement | Design component | Source file | Test | Verification evidence | Status |
|---|---|---|---|---|---|
| `CAD2SIM-REQ-001` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-002` | `include/cad2sim/math/vec3.hpp` | `src/vec3.cpp` | `tests/unit/math/test_vec3.cpp`, `tests/unit/geometry/test_advanced_primitives.cpp` | Deterministic numerical assertions cover vectors, dot/cross products, normalization, distances, projections, and coordinate-frame conversion with tolerance checks; current full CTest: 10/10 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-003` | `include/cad2sim/geometry/primitives.hpp` | `tests/unit/geometry/test_primitives.cpp`, `tests/unit/geometry/test_advanced_primitives.cpp` | Point3, Line3, Plane3, Circle3, PlaneSurface3 and oriented Frame3 | CTest: 5/5 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-004` | `include/cad2sim/kernel/geometry_kernel.hpp` | `src/kernel/geometry_kernel.cpp` | `tests/integration/kernel/test_step_import.cpp` | Representative STEP import and B-Rep validity; deterministic missing-file failure; CTest: `cad2sim_step_import_integration` passed; current full CTest: 10/10 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-005` | `include/cad2sim/kernel/geometry_kernel.hpp` | `src/kernel/geometry_kernel.cpp` | `tests/integration/topology/test_brep_topology.cpp` | B-Rep traversal of solids, shells, faces, wires, edges, vertices; dedicated CTest passed; current full CTest: 10/10 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-006` | `include/cad2sim/kernel/geometry_kernel.hpp` | `src/kernel/geometry_kernel.cpp`, `src/kernel/geometry_validation.hpp` | Valid STEP geometry validation; intentionally invalid B-Rep validation; deterministic diagnostic output; dedicated CTest passed; current full CTest: 10/10 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-007` | `include/cad2sim/kernel/geometry_kernel.hpp` | `src/kernel/geometry_kernel.cpp` | Analytical 10x20x30 box and representative STEP property extraction covering bounding boxes, surface normals, areas, edge lengths, volume, and centroids; dedicated CTest passed; current full CTest: 10/10 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-008` | Feature Recognition Layer | `include/cad2sim/features/feature_recognition.hpp`, `src/features/feature_recognition.cpp` | `tests/integration/features/test_feature_recognition.cpp` | Controlled STEP fixtures cover planar faces, cylindrical faces, holes, pockets, fillets, and chamfers; negative controls cover external bosses, sharp/plain edges, hole-versus-pocket differentiation, and an unsupported spherical surface; explicit `Unknown/Unsupported` reporting with diagnostic; focused feature-recognition test passed; current full CTest: 10/10 passed, 0 failed; ambiguous-case classification remains unverified | `PENDING` |
| `CAD2SIM-REQ-009` | `include/cad2sim/math/transform3.hpp` | `src/transform3.cpp` | `tests/unit/math/test_transform3.cpp`, `tests/unit/geometry/test_advanced_primitives.cpp` | Deterministic tests cover identity, translation, Z-axis rotation, composition, rotation norm preservation, inverse composition, rigid-transform round-trips, and Frame3 local/world coordinate conversion; Transform3 provides translation, Z-axis rotation, affine application, rigid-transform inversion and composition; scaling is not implemented | `VERIFIED` |
| `CAD2SIM-REQ-010` | `include/cad2sim/kernel/geometry_kernel.hpp` | `src/kernel/geometry_kernel.cpp`, `src/features/feature_recognition.cpp` | `tests/integration/features/test_feature_recognition.cpp` | Dependency-boundary audit: no OCCT references in public CAD2Sim headers or feature-recognition sources; OCCT dependencies of `cad2sim_core` are CMake `PRIVATE`; clean build passed; full CTest: 10/10 passed, 0 failed | `VERIFIED` |
| `CAD2SIM-REQ-011` | `ValidatedShape` + `SurfaceMesher` | `include/cad2sim/kernel/validated_shape.hpp`, `include/cad2sim/mesh/surface_mesh.hpp`, `include/cad2sim/mesh/surface_mesher.hpp`, `src/kernel/validated_shape.cpp`, `src/kernel/detail/validated_shape_data.cpp`, `src/kernel/detail/step_shape_loader.cpp`, `src/mesh/surface_mesher.cpp` | `tests/integration/mesh/test_surface_mesh.cpp` | Validated STEP geometry is converted to triangular surface mesh data using OCCT `BRepMesh_IncrementalMesh`; deterministic repeated generation; invalid-parameter, invalid-shape and missing-STEP failure handling; mesh-node coordinate and triangle-index validation; full CTest: 11/11 passed, 0 failed; verified scope is surface meshing only | `VERIFIED` |
| `CAD2SIM-REQ-012` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-013` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-014` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-015` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-016` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-017` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-018` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-019` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-REQ-020` | TBD | TBD | TBD | TBD | `PENDING` |

### Non-Functional Requirements

| Requirement | Design component | Source file | Test | Verification evidence | Status |
|---|---|---|---|---|---|
| `CAD2SIM-NFR-001` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-002` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-003` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-004` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-005` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-006` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-007` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-008` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-009` | TBD | TBD | TBD | TBD | `PENDING` |
| `CAD2SIM-NFR-010` | TBD | TBD | TBD | TBD | `PENDING` |

### Traceability Rules

1. A requirement remains `PENDING` until implementation and verification evidence exist.
2. `IMPLEMENTED` shall only be used when corresponding implementation exists but verification is incomplete.
3. `VERIFIED` shall only be used when implementation and verification evidence are both available.
4. Source-file and test references shall point to real repository artifacts.
5. Verification evidence shall be based on reproducible project results.
6. Planned architecture shall not be presented as implemented functionality.
7. Professional experience with commercial geometry kernels shall not be inferred from architectural compatibility with those kernels.
