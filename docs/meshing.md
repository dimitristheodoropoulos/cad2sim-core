# CAD2Sim-Core Meshing Model

## 1. Purpose

This document defines the initial meshing architecture and engineering model for CAD2Sim-Core.

The purpose is to establish how validated CAD geometry and B-Rep topology shall be transformed into finite-element mesh representations suitable for downstream engineering analysis.

This document is a design baseline. The concepts described here shall not be considered implemented until corresponding source code and verification evidence exist.

---

## 2. Meshing Scope

The initial meshing scope covers:

* mesh preparation from CAD geometry
* surface mesh generation
* volume mesh generation
* mesh entity representation
* node and element connectivity
* physical/engineering region association
* mesh quality evaluation
* mesh validation
* deterministic mesh processing
* preparation of mesh data for downstream engineering analysis

The project focuses on the preprocessing layer.

It does not attempt to implement a complete finite-element solver.

---

## 3. Meshing Pipeline

The initial meshing workflow shall follow the conceptual pipeline:

```text
Validated CAD/B-Rep
        │
        ▼
Geometry Preparation
        │
        ├── Geometry validation
        ├── Small-feature handling
        ├── Tolerance handling
        └── Region identification
        │
        ▼
Surface Meshing
        │
        ▼
Surface Mesh Validation
        │
        ▼
Volume Meshing
        │
        ▼
Volume Mesh Validation
        │
        ▼
Mesh Quality Evaluation
        │
        ▼
Engineering Mesh Model
        │
        ▼
Downstream Analysis
```

Each stage shall have explicit inputs, outputs, and validation criteria.

---

## 4. Geometry Preparation for Meshing

CAD geometry shall be validated before meshing.

Preparation may include:

* checking topological validity
* identifying open or disconnected shells
* identifying degenerate geometry
* identifying very small geometric features
* applying configured geometric tolerances
* identifying faces suitable for meshing
* identifying regions that require special treatment

Geometry preparation shall not silently modify the source CAD model.

Any transformation or repair applied specifically for meshing shall be represented as an explicit preprocessing operation.

---

## 5. Surface Meshing

Surface meshing converts geometric faces into a discrete representation consisting of nodes and surface elements.

The initial implementation shall support triangular surface elements.

Conceptually:

```text
CAD Face
   │
   ▼
Discretization
   │
   ▼
Surface Nodes
   │
   ▼
Surface Elements
```

Surface mesh generation shall preserve the relationship between mesh entities and their originating CAD faces where the underlying meshing system permits this.

This association is important for engineering-region identification and boundary-condition preparation.

---

## 6. Volume Meshing

For closed volumetric regions, the preprocessing pipeline shall support generation of a three-dimensional volume mesh.

The initial target element type shall be tetrahedral elements.

Conceptually:

```text
Closed CAD Solid
       │
       ▼
Surface Mesh
       │
       ▼
Volume Discretization
       │
       ▼
Tetrahedral Volume Mesh
```

Volume meshing shall require a suitable closed or otherwise explicitly supported volumetric region.

Open or invalid geometry shall not be silently treated as valid volumetric input.

---

## 7. Mesh Data Model

The project-level mesh representation shall distinguish nodes, elements, and engineering regions.

Conceptually:

```text
Mesh
 ├── Nodes
 ├── Surface Elements
 ├── Volume Elements
 └── Regions
```

---

### 7.1 Mesh Node

A mesh node represents a discrete point in three-dimensional space.

A node shall conceptually contain:

* stable node identifier
* Cartesian coordinates
* optional originating CAD reference

Node identifiers shall be deterministic within a generated mesh where practical.

---

### 7.2 Surface Element

A surface element represents a discretized portion of a CAD surface.

The initial target representation is a triangle:

```text
      n1
     /  \
    /    \
   n2----n3
```

A surface element shall contain:

* stable element identifier
* node connectivity
* originating CAD face or region reference where available
* element type

---

### 7.3 Volume Element

A volume element represents a discretized portion of a three-dimensional region.

The initial target representation is a tetrahedron.

A volume element shall contain:

* stable element identifier
* node connectivity
* originating engineering region reference
* element type

---

### 7.4 Engineering Region

A mesh region associates discrete mesh entities with an engineering interpretation.

Conceptually:

```text
Region
 ├── Region ID
 ├── Source CAD references
 ├── Surface elements
 ├── Volume elements
 └── Engineering metadata
```

A region may later be associated with:

* material
* boundary condition
* load
* component identity
* analysis-specific metadata

The initial project does not implement a complete solver-side boundary-condition or material system.

---

## 8. CAD-to-Mesh Association

A central preprocessing requirement is preservation of traceability between CAD entities and mesh entities.

The conceptual relationship is:

```text
CAD Face
    │
    ├───────────────┐
    ▼               ▼
Surface Region   Mesh Elements
                    │
                    ▼
             Engineering Region
```

Where supported, each generated mesh entity shall retain sufficient information to determine its originating CAD or engineering region.

This association shall be designed so that downstream preprocessing operations do not need to infer CAD ownership solely from spatial proximity.

---

## 9. Mesh Element Connectivity

Mesh connectivity shall be represented explicitly.

For a triangular surface element:

```text
Element E
 ├── Node 1
 ├── Node 2
 └── Node 3
```

For a tetrahedral volume element:

```text
Element E
 ├── Node 1
 ├── Node 2
 ├── Node 3
 └── Node 4
```

Connectivity shall reference valid mesh-node identifiers.

The implementation shall validate that:

* every referenced node exists
* element connectivity has the expected cardinality
* no invalid node identifiers are present
* element topology is internally consistent

---

## 10. Mesh Orientation

Element orientation shall be treated as an explicit preprocessing concern.

For surface elements, orientation determines the direction of the associated surface normal.

For volume elements, orientation affects the sign of the element Jacobian and therefore the validity of the element mapping.

The implementation shall define and verify orientation conventions.

Negative or degenerate volume orientation shall be detected during mesh validation.

---

## 11. Mesh Quality Metrics

Mesh quality shall be evaluated using explicit numerical metrics rather than visual inspection alone.

Initial target metrics include:

* element edge-length statistics
* aspect ratio
* minimum and maximum angles where applicable
* skewness where supported
* element area for surface elements
* element volume for volume elements
* Jacobian-related validity measures
* degenerate-element detection

Quality metrics shall be associated with defined thresholds where engineering interpretation requires them.

Thresholds shall be configurable rather than embedded as unexplained constants.

---

## 12. Surface Mesh Quality

Surface mesh validation shall identify problematic elements such as:

* zero-area triangles
* near-zero-area triangles
* excessive aspect ratio
* extreme angles
* duplicated nodes where detectable
* invalid connectivity
* inconsistent orientation where relevant

The validation result shall distinguish between:

```text
Valid
Warning
Invalid
Unsupported
```

A warning shall not automatically be treated as an invalid mesh.

---

## 13. Volume Mesh Quality

Volume mesh validation shall identify problematic tetrahedral elements such as:

* zero-volume elements
* near-zero-volume elements
* inverted elements
* degenerate elements
* excessive aspect ratio
* poor Jacobian quality
* invalid connectivity

An inverted or degenerate volume element shall be treated as a critical mesh-validation failure when the mesh is intended for downstream engineering analysis.

---

## 14. Mesh Validation

Mesh validation shall be a dedicated preprocessing stage.

Validation shall include, where applicable:

### Structural Validation

* node identifiers are valid
* element identifiers are valid
* connectivity references existing nodes
* element types match connectivity requirements
* region references are valid

### Geometric Validation

* node coordinates are finite
* surface elements have valid area
* volume elements have valid volume
* degenerate elements are detected

### Topological Validation

* shared entities are represented consistently
* surface/volume relationships are valid where represented
* region assignments are internally consistent

### Engineering Validation

* required engineering regions exist
* unsupported configurations are reported
* critical mesh-quality failures prevent downstream model generation

---

## 15. Mesh Generation Parameters

Meshing parameters shall be explicit and reproducible.

Initial parameters may include:

* global element size
* minimum element size
* maximum element size
* curvature-related refinement controls
* boundary-layer or local refinement settings where supported
* surface/volume meshing options

The exact parameter set shall depend on the selected meshing backend.

Backend-specific configuration shall not leak unnecessarily into higher-level engineering code.

---

## 16. Local Refinement

The meshing architecture shall support the concept of local refinement.

Potential refinement drivers include:

* geometric curvature
* small geometric features
* identified engineering features
* user-defined regions
* mesh-quality requirements

Conceptually:

```text
Global Mesh Size
        │
        ▼
Local Refinement Rules
        │
        ├── Feature A → finer mesh
        ├── Feature B → finer mesh
        └── Region C  → finer mesh
```

Local refinement is an architectural capability initially; concrete refinement algorithms shall be implemented only when supported by verification evidence.

---

## 17. Mesh Backend Abstraction

The project shall isolate mesh-generation functionality behind a project-level abstraction.

Conceptually:

```text
CAD2Sim-Core Meshing API
          │
          ▼
     Mesh Backend
          │
          ├── Gmsh Adapter
          └── Future Backend
```

The application and engineering-model layers shall not depend directly on backend-specific APIs where avoidable.

The initial development implementation is intended to use Gmsh as the meshing backend.

This backend selection is an implementation choice and does not constitute prior professional experience with any commercial meshing system.

---

## 18. Determinism and Reproducibility

The meshing pipeline shall aim for deterministic results for identical:

* input geometry
* geometry-kernel state
* meshing configuration
* preprocessing parameters
* backend version

Deterministic processing shall be verified where the selected backend permits it.

At minimum, the project shall verify deterministic higher-level summaries such as:

* node count
* surface-element count
* volume-element count
* region count
* quality-statistics summary

Exact node/element ordering shall only be treated as a deterministic requirement when it can be guaranteed by the backend and project configuration.

---

## 19. Performance Considerations

Meshing performance shall be evaluated as an engineering characteristic rather than optimized prematurely.

Potential measurements include:

* geometry preparation time
* surface mesh generation time
* volume mesh generation time
* mesh validation time
* total preprocessing time
* peak memory usage
* node count
* element count

Benchmarks shall use controlled reference models.

Performance results shall identify:

* hardware
* software versions
* model characteristics
* meshing parameters
* measured runtime

No performance claim shall be made without reproducible measurement evidence.

---

## 20. Failure Handling

Meshing failures shall be explicit and diagnosable.

Potential failure categories include:

```text
InvalidGeometry
UnsupportedGeometry
MeshingFailure
InvalidMesh
QualityThresholdViolation
BackendFailure
InvalidConfiguration
```

The implementation shall avoid converting backend failures into generic success/failure states that lose useful diagnostic information.

Where possible, errors shall include:

* processing stage
* affected entity or region
* backend diagnostic information
* relevant configuration
* recoverability classification

---

## 21. Engineering-Model Preparation

A validated mesh shall be convertible into a project-level engineering representation.

Conceptually:

```text
Validated Mesh
      │
      ▼
Engineering Mapping
      │
      ├── Regions
      ├── CAD references
      ├── Mesh references
      └── Analysis metadata
      │
      ▼
Engineering Model
```

The engineering model shall remain independent of the concrete meshing backend.

The initial implementation shall focus on representing preprocessing information required by downstream analysis rather than implementing the analysis solver itself.

---

## 22. Mesh Serialization

Mesh data may need to be serialized for:

* regression testing
* inspection
* benchmarking
* downstream processing
* reproducibility

The project-level representation shall prefer deterministic, explicit data structures.

Opaque backend-specific mesh objects shall not be used as the primary engineering representation.

Where a textual representation is used for regression, it shall avoid unnecessary backend-dependent metadata that prevents meaningful comparison.

---

## 23. Reference Models

Meshing verification shall use controlled reference geometries.

Initial reference models should include:

```text
Model A — Box
    Simple closed solid
    Baseline topology and mesh

Model B — Cylinder
    Curved surface
    Surface classification and volume meshing

Model C — Box with Hole
    Inner boundary
    Topology/feature association

Model D — Thin Feature
    Small characteristic dimension
    Refinement and quality behavior

Model E — Multi-Region Model
    Multiple engineering regions
    Region mapping and traceability
```

Additional models may be introduced as required by identified failure modes.

Reference models shall be version-controlled or otherwise reproducibly generated.

---

## 24. Meshing Verification

Verification shall be organized into controlled test categories.

### 24.1 Structural Verification

Verify:

* node counts
* element counts
* connectivity validity
* element-type consistency
* region references

### 24.2 Geometric Verification

Verify:

* surface-element areas
* volume-element volumes
* element orientation
* geometric bounds
* quality metrics

### 24.3 Topological Verification

Verify:

* CAD-to-mesh associations
* boundary representation relationships
* surface/volume connectivity
* region assignments

### 24.4 Robustness Verification

Verify behavior for:

* invalid geometry
* degenerate geometry
* unsupported geometry
* extreme mesh parameters
* small geometric features
* malformed backend output where detectable

### 24.5 Regression Verification

Verify repeated processing of identical reference models.

Regression evidence shall record relevant:

* software versions
* model identifiers
* meshing parameters
* mesh summaries
* quality summaries

No verification result is claimed at this stage.

---

## 25. Meshing Requirements Traceability

| Requirement       | Meshing-model evidence                             |
| ----------------- | -------------------------------------------------- |
| `CAD2SIM-REQ-004` | Meshing Pipeline                                   |
| `CAD2SIM-REQ-011` | Mesh Data Model                                    |
| `CAD2SIM-REQ-012` | CAD-to-Mesh Association                            |
| `CAD2SIM-REQ-013` | Engineering-Model Preparation / Mesh Serialization |
| `CAD2SIM-REQ-014` | Mesh Quality Metrics                               |
| `CAD2SIM-REQ-015` | Determinism and Reproducibility                    |
| `CAD2SIM-REQ-016` | Mesh Validation / Failure Handling                 |
| `CAD2SIM-REQ-017` | Meshing Verification                               |
| `CAD2SIM-REQ-018` | Regression Verification                            |
| `CAD2SIM-NFR-004` | Performance Considerations                         |
| `CAD2SIM-NFR-005` | Determinism and Reproducibility                    |
| `CAD2SIM-NFR-006` | Failure Handling                                   |
| `CAD2SIM-NFR-007` | Mesh Backend Abstraction                           |
| `CAD2SIM-NFR-008` | Mesh Serialization                                 |
| `CAD2SIM-NFR-009` | Performance Considerations                         |
| `CAD2SIM-NFR-010` | Engineering-Model Preparation                      |

This table represents design intent only. It does not establish implementation or verification status.

---

## 26. Relationship to Geometry Model

The meshing layer depends on the geometry model defined in `docs/geometry_model.md`.

The conceptual dependency is:

```text
Geometry Model
      │
      ▼
Geometry Validation
      │
      ▼
Meshing Preparation
      │
      ▼
Mesh Model
      │
      ▼
Engineering Model
```

The meshing layer shall consume project-level geometry concepts rather than exposing CAD-kernel-specific objects to higher-level application code.

---

## 27. Relationship to Architecture

The meshing design follows the architectural boundaries defined in `docs/architecture.md`.

The intended dependency direction is:

```text
Application
    │
    ▼
Engineering Model
    │
    ▼
Meshing
    │
    ▼
Geometry / Topology
    │
    ▼
Geometry Kernel
```

Backend-specific implementation details shall remain below the corresponding abstraction boundaries.

---

## 28. Out of Scope

The following are outside the initial CAD2Sim-Core meshing scope:

* complete finite-element solver implementation
* nonlinear material solvers
* CFD solver implementation
* electromagnetic solver implementation
* optimization algorithms
* adaptive solver-driven remeshing
* production-grade automatic mesh repair for arbitrary CAD
* commercial meshing-kernel compatibility claims
* industrial certification
* physical experimental validation

These capabilities may be considered future extensions but shall not be implied by the initial project.

---

## 29. Current Status

Meshing design baseline defined.

The architecture specifies the intended relationship between CAD geometry, surface/volume meshing, mesh quality evaluation, validation, and engineering-model preparation.

Implementation has not yet started.

No mesh-generation, mesh-quality, performance, or determinism result is claimed at this stage.

The initial meshing backend is intended to be Gmsh, subject to dependency and implementation validation.

All future implementation and verification claims shall be supported by concrete source code, automated tests, and reproducible evidence.