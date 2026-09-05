# CAD2Sim-Core Verification Strategy

## 1. Purpose

This document defines the verification strategy for CAD2Sim-Core.

The purpose is to establish how the project shall demonstrate that its mathematical, geometric, topological, meshing, and engineering-preprocessing behavior is correct, robust, deterministic, and reproducible.

Verification shall be based on executable tests and measurable evidence rather than documentation claims or visual inspection alone.

This document is a verification design baseline. No verification result is claimed at this stage.

---

## 2. Verification Scope

Verification shall cover the following major areas:

* 3D mathematical operations
* coordinate transformations
* geometric properties
* geometric classification
* B-Rep topology
* topology/geometry relationships
* geometry validation
* feature recognition
* surface meshing
* volume meshing
* mesh connectivity
* mesh orientation
* mesh-quality evaluation
* CAD-to-mesh traceability
* engineering-model generation
* deterministic processing
* failure handling
* regression behavior
* performance measurements

The project shall distinguish between:

```text
Implementation
     │
     ▼
Verification
     │
     ▼
Evidence
```

An implemented component shall not be marked VERIFIED until appropriate verification evidence exists.

---

## 3. Verification Principles

CAD2Sim-Core shall follow these verification principles:

1. requirements drive verification
2. tests shall be reproducible
3. numerical comparisons shall use explicit tolerances
4. negative and invalid-input cases shall be tested
5. geometry and topology shall be verified separately where appropriate
6. integration behavior shall be tested across component boundaries
7. deterministic behavior shall be verified where technically meaningful
8. performance claims shall be based on measured data
9. test evidence shall identify the exact software baseline
10. verification status shall never be inferred from implementation alone

---

## 4. Verification Levels

Verification shall be organized into multiple levels.

```text
Unit Tests
    │
    ▼
Component Tests
    │
    ▼
Geometry/Topology Integration Tests
    │
    ▼
Meshing Integration Tests
    │
    ▼
End-to-End Preprocessing Tests
    │
    ▼
Regression / Benchmark Evidence
```

Each level addresses different failure modes.

---

## 5. Unit Verification

Unit tests shall verify isolated project-level components.

Initial unit-test categories include:

### Mathematics

* vector addition/subtraction
* scalar operations
* dot product
* cross product
* vector magnitude
* normalization
* point/vector relationships
* distances
* transformations
* transformation composition
* inverse transformations

### Geometry

* geometric classification
* bounding-box calculations
* area calculations
* volume calculations
* centroid calculations
* tolerance-sensitive comparisons

### Validation

* valid entities
* invalid entities
* degenerate entities
* unsupported entities
* invalid configurations

Unit tests shall minimize dependency on external CAD files where a mathematical or algorithmic component can be tested independently.

---

## 6. Component Verification

Component-level tests shall verify behavior at project abstraction boundaries.

Initial component boundaries include:

```text
Math
Geometry
Topology
Kernel
Features
Meshing
Engineering
I/O
```

Examples include:

* geometry-to-topology conversion
* kernel adapter behavior
* topology traversal
* feature classification
* mesh generation configuration
* mesh-quality evaluation
* engineering-region construction
* serialization/deserialization where supported

Component tests shall verify both successful and failure paths.

---

## 7. Reference Geometry Strategy

Verification shall use controlled reference geometries with known expected properties.

Initial reference cases shall include:

```text
Reference A — Unit Box
Reference B — Rectangular Box
Reference C — Cylinder
Reference D — Sphere
Reference E — Box with Through Hole
Reference F — Box with Pocket
Reference G — Thin Feature
Reference H — Multi-Region Solid
Reference I — Invalid/Open Geometry
Reference J — Degenerate/Problematic Geometry
```

Each reference model shall have documented expected characteristics.

Where analytical solutions are available, expected values shall be derived independently of the implementation under test.

---

## 8. Analytical Geometry Verification

Analytical geometries shall be used to verify fundamental geometric calculations.

Examples:

### Box

Expected properties may include:

* exact dimensions within tolerance
* analytical volume
* analytical surface area
* predictable bounding box
* predictable centroid

### Cylinder

Expected properties may include:

* radius
* height
* analytical volume
* analytical lateral/surface area
* bounding box
* surface classification

### Sphere

Expected properties may include:

* radius
* analytical volume
* analytical surface area
* centroid
* surface classification

Analytical reference values shall be calculated independently from the implementation.

---

## 9. Numerical Tolerance Strategy

Geometric verification shall use explicit numerical tolerances.

The verification framework shall distinguish conceptually between:

* absolute tolerance
* relative tolerance
* geometric modeling tolerance
* topology-validation tolerance
* classification tolerance
* mesh-quality tolerance

For a value comparison:

```text
|actual - expected| <= tolerance
```

shall be preferred to exact floating-point equality.

For quantities whose scale varies significantly, a combined absolute/relative comparison may be used.

Tolerance values shall be documented for each verification category where appropriate.

---

## 10. Floating-Point Robustness

Verification shall explicitly consider numerical edge cases.

Examples include:

* near-zero vectors
* nearly parallel vectors
* nearly coincident points
* very small geometric features
* large coordinate magnitudes
* nearly degenerate triangles
* nearly zero-volume tetrahedra
* tolerance-boundary conditions

The purpose is to detect algorithms that work only for ideal numerical inputs.

---

## 11. Transformation Verification

Coordinate transformations shall be verified independently.

Initial tests shall include:

* identity transformation
* pure translation
* pure rotation
* composed transformations
* inverse transformations
* point transformation
* vector transformation
* round-trip transformation

For a point `P` and transformation `T`:

```text
T^-1(T(P)) ≈ P
```

shall hold within the defined numerical tolerance.

Rigid transformations shall also preserve:

* distances
* angles where applicable
* relative geometric relationships

---

## 12. B-Rep Topology Verification

B-Rep verification shall validate topology independently from geometric property calculations.

Initial checks shall include:

* solid count
* shell count
* face count
* wire count
* edge count
* vertex count
* face-to-edge relationships
* edge-to-vertex relationships
* face adjacency
* shell closure
* solid closure
* orientation information

Reference models shall be selected so that expected topology can be independently reasoned about.

---

## 13. Geometry/Topology Consistency

Verification shall confirm consistency between geometry and topology.

Examples include:

```text
Vertex → geometric position
Edge   → underlying curve
Face   → underlying surface
Solid  → bounded volumetric region
```

Tests shall verify that topological entities reference appropriate geometric entities where such relationships are required.

The verification suite shall also test missing, invalid, or inconsistent relationships where the backend permits their construction or detection.

---

## 14. Geometry Validation Verification

Geometry validation shall be tested using both valid and invalid reference cases.

Expected validation categories include:

```text
Valid
Warning
Invalid
Unsupported
```

Tests shall verify that:

* valid geometry is accepted
* invalid geometry is rejected when required
* warnings do not become silent failures
* unsupported geometry is explicitly reported
* critical validation failures stop downstream processing where required

Validation diagnostics shall be checked where their contents are part of the project contract.

---

## 15. Feature Recognition Verification

Feature recognition shall be verified against controlled models.

Initial target features include:

* planar regions
* cylindrical regions
* holes
* pockets
* fillets
* chamfers

Verification shall test both:

```text
Feature Present
Feature Absent
```

False-positive behavior shall be treated as a first-class verification concern.

A candidate feature shall not be considered recognized solely because one geometric property matches.

Recognition shall consider relevant combinations of:

* geometry
* topology
* dimensions
* adjacency
* orientation

---

## 16. Surface Meshing Verification

Surface meshing shall be verified using controlled reference geometries.

Initial checks shall include:

* mesh generation succeeds for valid input
* node coordinates are finite
* surface element connectivity is valid
* surface element count is non-zero where expected
* surface elements reference existing nodes
* surface element areas are valid
* degenerate elements are detected
* CAD-face associations are preserved where supported

Analytical surfaces shall provide useful reference cases for checking discretization behavior.

---

## 17. Volume Meshing Verification

Volume meshing shall be verified using closed reference solids.

Initial checks shall include:

* volume mesh generation succeeds for valid closed solids
* tetrahedral connectivity is valid
* referenced nodes exist
* element volumes are positive where required
* inverted elements are detected
* degenerate elements are detected
* volume mesh remains inside the intended region where this can be reliably verified
* CAD/engineering-region associations are preserved where supported

Open or invalid geometry shall be tested as negative cases.

---

## 18. Mesh Quality Verification

Mesh-quality metrics shall be verified using controlled element configurations.

The test suite shall include deliberately constructed or generated cases representing:

* well-shaped elements
* high-aspect-ratio elements
* near-degenerate elements
* zero-area triangles
* zero-volume tetrahedra
* inverted tetrahedra

The purpose is to verify that the quality evaluator correctly distinguishes acceptable and problematic elements.

Quality calculations shall be compared against independently calculated reference values where practical.

---

## 19. Mesh Connectivity Verification

Mesh connectivity shall be validated structurally.

Tests shall verify:

* every element references existing nodes
* connectivity cardinality is correct
* no invalid node identifiers exist
* element identifiers are unique within the mesh
* node identifiers are unique within the mesh
* region references are valid
* shared entities are represented consistently where required

Malformed connectivity shall result in deterministic validation failures.

---

## 20. Mesh Orientation Verification

Orientation shall be verified explicitly.

For surface elements:

* normal direction shall follow the defined convention

For tetrahedral elements:

* signed volume/Jacobian shall satisfy the defined orientation convention

Verification shall include:

* correctly oriented elements
* inverted elements
* degenerate elements

Orientation failures shall not be silently accepted.

---

## 21. CAD-to-Mesh Traceability Verification

The relationship between CAD entities and generated mesh entities shall be verified.

Tests shall verify, where supported:

```text
CAD Face
   ↓
Surface Mesh Region
   ↓
Mesh Elements
```

and:

```text
CAD Solid
   ↓
Engineering Region
   ↓
Volume Elements
```

The verification suite shall confirm that traceability remains valid after serialization and reloading where serialization is supported.

---

## 22. Engineering-Model Verification

The engineering-model layer shall be verified independently from the meshing backend.

Tests shall verify:

* region creation
* stable region identifiers
* source CAD references
* mesh references
* metadata consistency
* serialization behavior
* rejection of invalid references
* deterministic output where required

The engineering model shall not depend on opaque backend objects.

---

## 23. Serialization Verification

Where project-level serialization is implemented, tests shall include:

* serialize valid model
* deserialize valid model
* round-trip equivalence
* malformed input
* missing required fields
* invalid identifiers
* unsupported versions
* deterministic serialization ordering where required

A successful round trip shall preserve all contractually relevant information.

---

## 24. Failure-Path Verification

Negative testing shall be treated as equally important as successful-path testing.

Initial failure categories include:

```text
InvalidInput
InvalidGeometry
UnsupportedGeometry
TopologyFailure
FeatureRecognitionFailure
MeshingFailure
InvalidMesh
QualityThresholdViolation
SerializationFailure
InvalidConfiguration
BackendFailure
```

Tests shall verify:

* correct failure classification
* meaningful diagnostic information
* no silent corruption
* no partially valid engineering model where prohibited
* deterministic failure behavior where appropriate

---

## 25. Integration Verification

Integration tests shall verify complete component boundaries.

Initial integration paths include:

```text
CAD File
   ↓
Kernel Adapter
   ↓
Geometry/Topology
   ↓
Validation
   ↓
Feature Analysis
   ↓
Meshing
   ↓
Mesh Validation
   ↓
Engineering Model
```

Integration tests shall use real project components rather than mocks wherever practical.

Mocks or stubs may be used when isolating unavailable or intentionally external behavior.

---

## 26. End-to-End Verification

End-to-end tests shall process complete reference models through the preprocessing pipeline.

At minimum, end-to-end verification shall cover:

* CAD import
* geometry validation
* topology inspection
* feature processing where applicable
* surface meshing
* volume meshing
* mesh validation
* engineering-model generation
* output serialization where implemented

Each end-to-end test shall identify:

* input model
* configuration
* software baseline
* expected result
* measured result

---

## 27. Regression Strategy

The project shall maintain a regression suite containing stable reference cases.

Regression tests shall detect unintended changes in:

* topology summaries
* geometric properties
* feature-recognition results
* mesh counts
* mesh-quality statistics
* engineering-region assignments
* serialized engineering output
* failure classifications

Regression baselines shall be version-controlled where practical.

A regression change shall require investigation rather than automatic acceptance.

---

## 28. Determinism Verification

Determinism shall be tested by processing identical inputs repeatedly under identical configurations.

The test shall compare relevant outputs such as:

* entity counts
* geometric-property summaries
* feature summaries
* node counts
* element counts
* region counts
* quality statistics
* engineering-model summaries

Exact ordering shall only be required where deterministic ordering is part of the project contract.

The verification environment shall record relevant:

* compiler version
* library versions
* backend versions
* operating-system information
* configuration parameters

---

## 29. Performance Verification

Performance shall be measured using controlled benchmark models.

Initial benchmark metrics include:

* CAD import time
* geometry-analysis time
* topology traversal time
* feature-recognition time
* surface-meshing time
* volume-meshing time
* mesh-validation time
* engineering-model generation time
* total preprocessing time
* peak memory usage

Benchmark results shall include:

* hardware
* operating system
* compiler
* build configuration
* dependency versions
* input model characteristics
* meshing parameters
* measured runtime
* memory measurement method

Performance results shall not be generalized beyond the tested environment without appropriate qualification.

---

## 30. Performance Regression

Performance regression testing shall compare benchmark results against established project baselines.

Potential regression indicators include:

```text
Runtime increase
Memory increase
Unexpected mesh-size increase
Unexpected topology-processing increase
```

Thresholds shall be configurable and documented.

Performance regressions shall trigger investigation rather than being silently accepted.

---

## 31. Test Data Management

Reference CAD models and expected verification data shall be treated as controlled test assets.

Each reference model should have:

* stable identifier
* description
* expected geometric characteristics
* expected topology characteristics where known
* expected feature characteristics where applicable
* expected meshing configuration
* expected verification outcomes

Test assets shall be version-controlled where licensing permits.

Third-party CAD files shall only be committed when their redistribution rights permit it.

Where redistribution is not permitted, deterministic generation scripts or documented acquisition procedures shall be preferred.

---

## 32. Reproducibility

Verification shall be reproducible from a clean checkout.

The repository shall document:

* compiler version
* build configuration
* required dependencies
* test commands
* benchmark commands
* reference-model generation
* expected outputs

A verification result shall identify the Git commit or equivalent immutable software baseline against which it was produced.

---

## 33. Verification Evidence

Each VERIFIED requirement shall have evidence sufficient for an independent reviewer to reproduce the result.

Evidence may include:

* source-code reference
* test name
* test output
* reference model
* configuration
* measured value
* expected value
* tolerance
* benchmark result
* regression result

Evidence shall not rely solely on screenshots or undocumented manual inspection.

---

## 34. Requirements Traceability

Verification status shall be linked to the requirements defined in `docs/requirements.md`.

The intended relationship is:

```text
Requirement
    │
    ▼
Design Component
    │
    ▼
Implementation
    │
    ▼
Test
    │
    ▼
Verification Evidence
```

A requirement shall be considered:

```text
PENDING
```

until implementation and verification evidence exist.

It may be considered:

```text
IMPLEMENTED
```

when the corresponding implementation exists but verification is incomplete.

It may be considered:

```text
VERIFIED
```

only when the implementation and appropriate verification evidence both exist.

---

## 35. Verification Record Format

Future verification records should capture at least:

```text
Requirement ID:
Verification ID:
Software baseline:
Test name:
Input/reference model:
Configuration:
Expected result:
Actual result:
Tolerance:
Pass/Fail:
Evidence:
```

For performance verification:

```text
Hardware:
Operating system:
Compiler:
Build type:
Dependency versions:
Input model:
Runtime:
Peak memory:
Result:
```

The exact final format may evolve during implementation.

---

## 36. Test Naming and Organization

Tests shall use stable, descriptive names.

Conceptually:

```text
tests/
├── unit/
│   ├── math/
│   ├── geometry/
│   ├── topology/
│   ├── validation/
│   └── engineering/
├── integration/
│   ├── kernel/
│   ├── features/
│   ├── meshing/
│   └── pipeline/
├── regression/
└── benchmarks/
```

The final directory structure may be adjusted to the implementation framework.

Test names shall describe the behavior being verified rather than implementation details alone.

---

## 37. Continuous Integration

Continuous integration shall execute the verification suite that is practical for automated environments.

Initial CI goals include:

* clean build
* unit tests
* component tests
* integration tests
* regression tests
* static analysis where practical

Long-running geometry or meshing benchmarks may be separated from the normal CI gate when execution cost makes this appropriate.

CI shall report failures with enough information to reproduce them locally.

---

## 38. Verification of External Dependencies

External CAD and meshing libraries shall not be treated as implicitly correct.

The project shall verify the behavior that it relies upon at its abstraction boundaries.

Examples include:

* imported topology is interpreted correctly
* geometry properties are mapped correctly
* mesh entities are converted correctly
* backend failures are propagated correctly
* backend-specific identifiers are not incorrectly exposed as project-level identifiers

The project shall not claim verification of the complete internal correctness of an external library.

---

## 39. Verification Limitations

The following limitations shall be explicitly recognized:

* numerical geometry is subject to floating-point limitations
* CAD-kernel behavior may depend on kernel version
* meshing output may vary between backend versions
* exact mesh ordering may not be guaranteed
* some invalid geometry conditions may be difficult to detect completely
* feature recognition may be heuristic
* performance measurements depend on hardware and software environment

Verification shall therefore state its assumptions and scope.

---

## 40. Verification Status Convention

The project shall use the following status convention:

| Status         | Meaning                                                                                     |
| -------------- | ------------------------------------------------------------------------------------------- |
| `PENDING`      | Design requirement exists but implementation/verification evidence is not complete          |
| `IMPLEMENTED`  | Corresponding implementation exists but verification is incomplete                          |
| `VERIFIED`     | Implementation and verification evidence are both available                                 |
| `BLOCKED`      | Verification cannot currently be completed because of a documented dependency or limitation |
| `OUT-OF-SCOPE` | Requirement is explicitly outside the project scope                                         |

Status changes shall be supported by repository evidence.

---

## 41. Verification-to-Architecture Traceability

| Verification Area   | Architecture Evidence                                 |
| ------------------- | ----------------------------------------------------- |
| Mathematics         | `docs/architecture.md` — 3D Mathematics Foundation    |
| Geometry            | `docs/architecture.md` — Geometry Analysis            |
| Topology            | `docs/architecture.md` — B-Rep Processing Model       |
| Kernel              | `docs/architecture.md` — Geometry Kernel Boundary     |
| Feature recognition | `docs/architecture.md` — Feature Recognition Strategy |
| Meshing             | `docs/architecture.md` — Meshing Architecture         |
| Engineering model   | `docs/architecture.md` — Engineering Model Generation |
| Testing             | `docs/architecture.md` — Testing Architecture         |
| Performance         | `docs/architecture.md` — Performance Architecture     |
| Extensibility       | `docs/architecture.md` — Extensibility                |

This table describes verification alignment with the architecture. It does not constitute verification evidence.

---

## 42. Verification-to-Geometry Traceability

| Verification Area    | Geometry Model Evidence                                             |
| -------------------- | ------------------------------------------------------------------- |
| 3D mathematics       | `docs/geometry_model.md` — Fundamental Mathematical Entities        |
| Transformations      | `docs/geometry_model.md` — Coordinate and Reference-System Strategy |
| Geometric entities   | `docs/geometry_model.md` — Geometric Entities                       |
| B-Rep topology       | `docs/geometry_model.md` — B-Rep Topology                           |
| Adjacency            | `docs/geometry_model.md` — Topological Relationships                |
| Orientation          | `docs/geometry_model.md` — Orientation                              |
| Geometric properties | `docs/geometry_model.md` — Geometric Properties                     |
| Classification       | `docs/geometry_model.md` — Geometric Classification                 |
| Features             | `docs/geometry_model.md` — Feature-Level Geometry                   |
| Validation           | `docs/geometry_model.md` — Geometry Validation                      |
| Identity             | `docs/geometry_model.md` — Entity Identity                          |

This table represents verification planning only.

---

## 43. Verification-to-Meshing Traceability

| Verification Area       | Meshing Model Evidence                              |
| ----------------------- | --------------------------------------------------- |
| Meshing pipeline        | `docs/meshing.md` — Meshing Pipeline                |
| Surface meshing         | `docs/meshing.md` — Surface Meshing                 |
| Volume meshing          | `docs/meshing.md` — Volume Meshing                  |
| Mesh data model         | `docs/meshing.md` — Mesh Data Model                 |
| CAD-to-mesh association | `docs/meshing.md` — CAD-to-Mesh Association         |
| Connectivity            | `docs/meshing.md` — Mesh Element Connectivity       |
| Orientation             | `docs/meshing.md` — Mesh Orientation                |
| Quality                 | `docs/meshing.md` — Mesh Quality Metrics            |
| Validation              | `docs/meshing.md` — Mesh Validation                 |
| Determinism             | `docs/meshing.md` — Determinism and Reproducibility |
| Performance             | `docs/meshing.md` — Performance Considerations      |
| Failure handling        | `docs/meshing.md` — Failure Handling                |

This table represents verification planning only.

---

## 44. Gamma Role Requirements Mapping

The verification strategy is intentionally aligned with the technical themes of the target 3D Modeling engineering role.

| Role Requirement                     | Verification Evidence Planned                                         |
| ------------------------------------ | --------------------------------------------------------------------- |
| 3D mathematics                       | Mathematical unit tests and transformation verification               |
| Computational geometry               | Geometric-property, classification, tolerance, and robustness tests   |
| CAD geometry modeling                | CAD-kernel integration and B-Rep verification                         |
| ACIS/Parasolid-style kernel concepts | Kernel abstraction and adapter-boundary verification                  |
| Finite-element meshing               | Surface/volume mesh generation and mesh-quality verification          |
| Engineering preprocessing            | End-to-end CAD-to-engineering-model verification                      |
| C++                                  | Automated C++ unit/component/integration testing                      |
| Robust maintainable software         | Failure-path, regression, CI, and reproducibility verification        |
| Analytical/problem-solving skills    | Controlled reference cases and independently derived expected results |
| Performance                          | Controlled benchmarks and performance-regression analysis             |

The project does not claim professional ACIS or Parasolid experience.

The project does not claim 3+ years of professional experience based on project verification.

---

## 45. Verification Workflow

The implementation workflow shall follow:

```text
Requirement
    │
    ▼
Test Definition
    │
    ▼
RED
    │
    ▼
Implementation
    │
    ▼
GREEN
    │
    ▼
Regression
    │
    ▼
Evidence Capture
    │
    ▼
Requirement Status Update
```

Documentation shall be updated only after the corresponding repository evidence exists.

---

## 46. Verification Checkpoint Policy

Before marking a milestone complete, the project shall verify:

```text
[ ] Requirements identified
[ ] Design documented
[ ] Implementation present
[ ] Unit tests present
[ ] Integration tests present where applicable
[ ] Negative cases tested
[ ] Numerical tolerances defined
[ ] Regression executed
[ ] Results recorded
[ ] Traceability updated
[ ] No unsupported claims introduced
```

A milestone shall not be considered fully verified when critical applicable checks remain incomplete.

---

## 47. Current Status

Verification strategy baseline defined.

The document establishes the intended verification methodology for:

* 3D mathematics
* computational geometry
* B-Rep topology
* geometry validation
* feature recognition
* finite-element meshing
* mesh quality
* engineering preprocessing
* regression
* determinism
* performance
* requirements traceability

Implementation has not yet started.

No test, pass, performance, determinism, or verification result is claimed at this stage.

Future verification status shall be based exclusively on reproducible repository evidence.

---

## 48. Verification Matrix

This section provides a complete mapping from each requirement defined in `docs/requirements.md` to its planned verification method, test location, expected evidence, and current status.

All requirements are initially `PENDING`. Status updates will be made as implementation and verification evidence become available.

| Requirement ID | Verification Method | Planned Test Area | Expected Evidence | Current Status |
|----------------|----------------------|-------------------|-------------------|----------------|
| **CAD2SIM-REQ-001** | Unit + Integration | `tests/unit/build`, `tests/integration/build` | Clean build, compiler diagnostics, test execution | PENDING |
| **CAD2SIM-REQ-002** | Unit | `tests/unit/math` | Numerical assertions for vector ops, dot/cross, norms, distances, projections, transformations | PENDING |
| **CAD2SIM-REQ-003** | Unit | `tests/unit/geometry/test_primitives.cpp`, `tests/unit/geometry/test_advanced_primitives.cpp` | Point3, Line3, Plane3, Circle3, PlaneSurface3 and oriented Frame3; local/world conversion and round-trip tests; CTest 5/5 passed | VERIFIED |
| **CAD2SIM-REQ-004** | Integration | `tests/integration/kernel/test_step_import.cpp`, `tests/fixtures/step/screw.step` | Representative STEP import, B-Rep validity validation, deterministic missing-file failure reporting; dedicated CTest passed; full CTest 6/6 passed | VERIFIED |
| **CAD2SIM-REQ-005** | Integration | `tests/integration/topology` | Traversal and counts of solids, shells, faces, wires, edges, vertices | PENDING |
| **CAD2SIM-REQ-006** | Integration + Negative | `tests/integration/validation` | Valid vs. invalid geometry cases, deterministic diagnostic output | PENDING |
| **CAD2SIM-REQ-007** | Unit + Integration | `tests/unit/geometry`, `tests/integration/properties` | Bounding boxes, normals, areas, lengths, volumes, centroids vs. analytical references | PENDING |
| **CAD2SIM-REQ-008** | Integration | `tests/integration/features` | Classification of planar, cylindrical, holes, pockets, fillets, chamfers; false-positive checks | PENDING |
| **CAD2SIM-REQ-009** | Unit | `tests/unit/math` | Translation, rotation, scaling, frame transformations; round-trip tests | PENDING |
| **CAD2SIM-REQ-010** | Architecture Review + Integration | `tests/integration/kernel` | Interface tests, dependency-boundary review, adapter substitution check | PENDING |
| **CAD2SIM-REQ-011** | Integration | `tests/integration/meshing` | Surface/volume mesh generation on representative geometries, failure handling | PENDING |
| **CAD2SIM-REQ-012** | Unit + Integration | `tests/unit/mesh`, `tests/integration/meshing` | Element count, aspect ratio, angles, skewness, Jacobian metrics on known-good/degraded meshes | PENDING |
| **CAD2SIM-REQ-013** | Integration | `tests/integration/engineering` | Region identification, stable references to originating geometry | PENDING |
| **CAD2SIM-REQ-014** | Integration | `tests/integration/engineering` | Schema validation, deterministic serialization, round-trip tests | PENDING |
| **CAD2SIM-REQ-015** | Regression | `tests/regression/determinism` | Repeated runs on identical input, compare summaries/outputs | PENDING |
| **CAD2SIM-REQ-016** | Negative Unit/Integration | `tests/unit/validation`, `tests/integration/negative` | Tests with unsupported geometry, invalid CAD, mesh failure; error reporting assertions | PENDING |
| **CAD2SIM-REQ-017** | CI + Regression | Entire test suite | Automated test execution, reproducible results, documented test logs | PENDING |
| **CAD2SIM-REQ-018** | Regression | `tests/regression` | Suite of primitives, topology variations, features, invalid cases, meshing, metadata | PENDING |
| **CAD2SIM-REQ-019** | Benchmark | `benchmarks/` | Timings, memory usage on representative models; documented methodology | PENDING |
| **CAD2SIM-REQ-020** | Documentation Review | N/A (docs) | All docs present: requirements, architecture, geometry, meshing, verification, limitations; traceability table | PENDING |
| **CAD2SIM-NFR-001** | Code Review + Architecture | N/A | Modular interfaces, clear separation of responsibilities | PENDING |
| **CAD2SIM-NFR-002** | Test Design | Test suite structure | Components testable independently (e.g., math, geometry without kernel) | PENDING |
| **CAD2SIM-NFR-003** | Build Verification | `CMakeLists.txt`, CI | Builds on target platforms without platform-specific assumptions | PENDING |
| **CAD2SIM-NFR-004** | Documentation + CI | `README`, `docs/`, CI scripts | Documented dependencies, repeatable build and test commands | PENDING |
| **CAD2SIM-NFR-005** | Negative Tests | `tests/integration/negative` | Failures produce actionable diagnostic info (e.g., error codes, messages) | PENDING |
| **CAD2SIM-NFR-006** | Architecture Review | N/A | CAD kernel, geometry, meshing, engineering, orchestration are separable | PENDING |
| **CAD2SIM-NFR-007** | Documentation | `README`, `requirements.md`, `architecture.md` | Explicit out-of-scope list, no production claims | PENDING |
| **CAD2SIM-NFR-008** | Documentation + CV | N/A | Clear distinction between project capability, academic knowledge, professional experience; no ACIS/Parasolid claims | PENDING |
| **CAD2SIM-NFR-009** | Process | Verification evidence | No capability marked VERIFIED without corresponding test evidence | PENDING |
| **CAD2SIM-NFR-010** | Documentation + Verification | Traceability tables | Every requirement maps to implementation and verification evidence | PENDING |

This verification matrix is a planning baseline. It will be updated as implementation progresses and evidence is collected.
