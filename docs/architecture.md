# CAD2Sim-Core Architecture

## 1. Purpose

This document defines the initial software architecture of CAD2Sim-Core.

The architecture is designed for a C++ engineering-software system that transforms 3D CAD geometry into simulation-oriented engineering data through a controlled preprocessing pipeline.

The architecture prioritizes:

* separation of concerns
* computational-geometry correctness
* testability
* maintainability
* geometry-kernel isolation
* deterministic processing
* explicit failure handling
* future extensibility

The architecture is a design baseline. Components described here shall not be considered implemented until corresponding source code and verification evidence exist.

---

## 2. Architectural Scope

CAD2Sim-Core covers the preprocessing path between imported CAD geometry and a structured engineering model.

```text
CAD Input
    │
    ▼
┌──────────────────────┐
│ CAD Import           │
│ STEP / future formats│
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ Geometry Kernel      │
│ Abstraction          │
└──────────┬───────────┘
           │
           ▼
┌──────────────────────┐
│ B-Rep / Geometry     │
│ Analysis             │
└──────────┬───────────┘
           │
           ├───────────────┐
           ▼               ▼
┌──────────────────┐  ┌──────────────────┐
│ Feature          │  │ Geometry         │
│ Recognition      │  │ Properties       │
└────────┬─────────┘  └────────┬─────────┘
         │                     │
         └──────────┬──────────┘
                    ▼
          ┌──────────────────────┐
          │ Mesh Preprocessing   │
          └──────────┬───────────┘
                     │
                     ▼
          ┌──────────────────────┐
          │ Mesh Quality         │
          │ Evaluation           │
          └──────────┬───────────┘
                     │
                     ▼
          ┌──────────────────────┐
          │ Engineering Model    │
          │ Generation           │
          └──────────────────────┘
```

The project does not include a complete CAD editor or a complete physics solver.

---

## 3. Architectural Layers

The system is divided into the following logical layers.

### 3.1 Application Layer

Responsible for orchestration of the preprocessing workflow.

Responsibilities:

* pipeline configuration
* execution sequencing
* input/output coordination
* error propagation
* command-line or future application interfaces

The application layer shall coordinate lower-level services but shall not implement geometry-kernel algorithms directly.

---

### 3.2 Engineering Model Layer

Responsible for the internal representation of simulation-oriented engineering data.

Responsibilities:

* engineering regions
* material references
* analysis entities
* boundary-condition-ready references
* preprocessing metadata
* deterministic serialization

This layer shall remain independent of vendor-specific CAD-kernel types.

---

### 3.3 Meshing Layer

Responsible for preparation and evaluation of finite-element mesh data.

Responsibilities:

* mesh generation orchestration
* surface and/or volume mesh handling
* mesh quality evaluation
* mesh diagnostics
* mesh-related preprocessing parameters

The meshing layer shall not contain CAD-kernel-specific application logic beyond the interfaces required to obtain valid geometry for meshing.

---

### 3.4 Feature Recognition Layer

Responsible for identifying engineering-relevant geometric features.

Initial target features include:

* planar faces
* cylindrical faces
* holes
* pockets
* fillets
* chamfers

Feature recognition shall operate through geometry abstractions rather than directly coupling higher-level application code to a specific CAD kernel.

Ambiguous or unsupported features shall be explicitly reported.

---

### 3.5 Geometry Analysis Layer

Responsible for computational-geometry operations and geometric property extraction.

Responsibilities include:

* bounding boxes
* lengths
* areas
* volumes
* centroids
* surface normals
* geometric classification
* geometric relationships
* coordinate transformations

This layer is the primary consumer of the 3D mathematics foundation.

---

### 3.6 Topology Layer

Responsible for B-Rep topology traversal and consistency analysis.

The logical topology hierarchy is:

```text
Solid
  └── Shell
       └── Face
            └── Wire
                 └── Edge
                      └── Vertex
```

Responsibilities include:

* topology traversal
* entity counting
* adjacency relationships
* connectivity analysis
* topology consistency checks
* stable identification of geometric entities where supported

The topology layer shall not expose vendor-specific kernel implementation details to unrelated application components.

---

### 3.7 Geometry Kernel Layer

The geometry-kernel layer isolates external CAD-kernel dependencies.

The initial implementation is intended to use OpenCASCADE Technology.

The application-facing architecture shall expose a project-defined abstraction rather than requiring higher-level components to depend directly on OpenCASCADE classes.

Conceptually:

```text
                 ┌─────────────────────┐
                 │ Geometry Kernel API  │
                 │ (CAD2Sim-Core)      │
                 └──────────┬──────────┘
                            │
                  ┌─────────┴─────────┐
                  │                   │
                  ▼                   ▼
          ┌───────────────┐   ┌────────────────┐
          │ OCC Adapter   │   │ Future Adapter │
          │               │   │                │
          │ OpenCASCADE   │   │ Commercial     │
          │               │   │ kernel         │
          └───────────────┘   └────────────────┘
```

A future commercial-kernel adapter may be introduced if licensing and technical requirements permit.

This architecture does not constitute professional experience with ACIS or Parasolid.

---

## 4. Dependency Direction

Dependencies shall follow a controlled direction.

```text
Application
    │
    ├──► Engineering Model
    ├──► Meshing
    ├──► Feature Recognition
    ├──► Geometry Analysis
    └──► Topology
             │
             ▼
      Geometry Kernel API
             │
             ▼
        Kernel Adapter
             │
             ▼
      External CAD Kernel
```

The preferred dependency rule is:

```text
Higher-level application logic
            ↓
Project-defined interfaces
            ↓
External technology adapters
```

External library types shall not unnecessarily propagate through the entire application.

---

## 5. Geometry Kernel Boundary

The geometry-kernel boundary is one of the primary architectural constraints.

Higher-level components should depend on concepts such as:

* `Point3D`
* `Vector3D`
* `Transform3D`
* `GeometryEntity`
* `TopologyEntity`
* `Surface`
* `Curve`
* `Solid`
* `BoundingBox`

rather than directly depending on vendor-specific classes.

The exact interfaces shall be refined during implementation.

The objective is not to create a complete replacement for a commercial CAD kernel. The objective is to prevent vendor-specific implementation details from becoming inseparable from the preprocessing application.

---

## 6. 3D Mathematics Foundation

The computational-geometry foundation shall provide reusable mathematical primitives.

Conceptual components include:

```text
Vector3D
Point3D
Matrix3D
Transform3D
Plane
CoordinateFrame
```

Core operations include:

* addition/subtraction
* scalar multiplication
* dot product
* cross product
* magnitude
* normalization
* distance
* projection
* transformation
* coordinate-frame conversion

Numerical operations shall use explicit tolerances where exact floating-point equality would be inappropriate.

---

## 7. B-Rep Processing Model

The B-Rep processing pipeline shall distinguish topology from geometry.

Conceptually:

```text
Topology Entity
      │
      ├── identity
      ├── adjacency
      └── connectivity
            │
            ▼
Geometry Entity
      │
      ├── curve
      ├── surface
      └── solid geometry
```

This separation is important because engineering preprocessing frequently requires both:

* topological relationships
* geometric properties

For example, a face may require both its surface classification and its adjacency relationship with neighboring faces.

---

## 8. Feature Recognition Strategy

Feature recognition shall be incremental.

The initial recognition pipeline is expected to follow:

```text
B-Rep Face
    │
    ▼
Geometric Classification
    │
    ├── Planar
    ├── Cylindrical
    └── Other
    │
    ▼
Topological Context
    │
    ▼
Feature Candidate
    │
    ▼
Feature Validation
    │
    ▼
Recognized Feature
```

Recognition algorithms shall distinguish between:

* confirmed features
* ambiguous candidates
* unsupported geometry

The system shall avoid silently classifying ambiguous geometry as a confirmed engineering feature.

---

## 9. Meshing Architecture

The meshing subsystem shall isolate mesh-generation technology from mesh-analysis logic.

Conceptually:

```text
CAD Geometry
     │
     ▼
Meshing Interface
     │
     ▼
Mesh Generator Adapter
     │
     ▼
Generated Mesh
     │
     ▼
Mesh Quality Analyzer
     │
     ├── Element count
     ├── Aspect ratio
     ├── Angle metrics
     ├── Skewness
     └── Jacobian-related metrics
```

The selected meshing technology shall be integrated behind an appropriate boundary where practical.

Mesh-quality calculations should operate on project-defined mesh representations where this improves testability and separation of concerns.

---

## 10. Engineering Model Generation

The engineering-model layer shall convert preprocessing results into a structured representation suitable for downstream simulation workflows.

Conceptual model:

```text
CAD Geometry
     │
     ▼
Recognized Features
     │
     ▼
Engineering Regions
     │
     ├── Region identity
     ├── Geometry references
     ├── Material reference
     └── Analysis metadata
     │
     ▼
Simulation-Oriented Model
```

The generated model is a preprocessing artifact.

It is not a physics-solver representation unless explicitly implemented later.

---

## 11. Error Handling

Errors shall be explicit and categorized where practical.

Expected categories include:

* invalid input
* unsupported CAD entity
* geometry validation failure
* topology inconsistency
* feature-recognition ambiguity
* meshing failure
* invalid preprocessing parameters
* serialization failure

Lower-level failures should retain sufficient diagnostic context when propagated upward.

Silent failure is prohibited for critical preprocessing stages.

---

## 12. Determinism

The preprocessing pipeline shall aim for deterministic behavior when:

* input geometry is identical
* processing parameters are identical
* the underlying external libraries provide deterministic behavior

Deterministic outputs are particularly important for:

* regression testing
* serialized engineering models
* feature-recognition results
* benchmark comparisons

Where external libraries introduce unavoidable nondeterminism, the limitation shall be documented rather than hidden.

---

## 13. Testing Architecture

Testing shall be layered.

```text
Unit Tests
    │
    ├── 3D mathematics
    ├── geometry primitives
    ├── transformations
    └── mesh metrics

Integration Tests
    │
    ├── CAD import
    ├── B-Rep traversal
    ├── feature recognition
    └── meshing

End-to-End Tests
    │
    └── CAD → Engineering Model
```

Tests shall use controlled reference geometries wherever practical.

Negative tests shall verify failure behavior rather than only successful processing.

---

## 14. Performance Architecture

Performance measurements shall be associated with identifiable processing stages.

Potential measurement boundaries include:

* CAD import
* topology extraction
* geometry analysis
* feature recognition
* mesh generation
* mesh-quality evaluation
* engineering-model serialization

Benchmark results shall record sufficient model-complexity information to make comparisons meaningful.

---

## 15. Extensibility

The architecture shall permit future extension in the following areas:

* additional CAD import formats
* alternative geometry kernels
* additional feature-recognition algorithms
* additional meshing technologies
* additional mesh-quality metrics
* richer engineering-model representations
* future graphical interfaces

Extensions shall prefer new implementations behind existing interfaces rather than modifications that unnecessarily couple unrelated subsystems.

---

## 16. Initial Component Boundaries

The following logical components are established as the initial design baseline:

| Component     | Primary responsibility                  | External dependency exposure    |
| ------------- | --------------------------------------- | ------------------------------- |
| `math`        | 3D mathematical primitives              | None preferred                  |
| `geometry`    | Geometric abstractions and properties   | Through kernel interfaces       |
| `topology`    | B-Rep traversal and relationships       | Through kernel interfaces       |
| `kernel`      | Geometry-kernel interfaces and adapters | OpenCASCADE                     |
| `features`    | Feature recognition                     | Project geometry/topology APIs  |
| `meshing`     | Mesh generation and analysis            | Meshing library through adapter |
| `engineering` | Engineering model representation        | None preferred                  |
| `application` | Pipeline orchestration                  | Project interfaces              |
| `io`          | Input/output and serialization          | Format libraries as required    |

These are logical boundaries. Physical source directories and class names will be finalized during implementation.

---

## 17. Architecture-to-Requirements Traceability

The initial architecture addresses the requirements as follows:

| Requirement       | Architectural evidence                                          |
| ----------------- | --------------------------------------------------------------- |
| `CAD2SIM-REQ-001` | C++ implementation target across all core layers                |
| `CAD2SIM-REQ-002` | 3D Mathematics Foundation                                       |
| `CAD2SIM-REQ-003` | Geometry Analysis + Geometry abstractions                       |
| `CAD2SIM-REQ-004` | CAD Import + Geometry Kernel Layer                              |
| `CAD2SIM-REQ-005` | Topology Layer + B-Rep Processing Model                         |
| `CAD2SIM-REQ-006` | Geometry Validation responsibilities                            |
| `CAD2SIM-REQ-007` | Geometry Analysis Layer                                         |
| `CAD2SIM-REQ-008` | Feature Recognition Layer                                       |
| `CAD2SIM-REQ-009` | 3D Mathematics Foundation + transformations                     |
| `CAD2SIM-REQ-010` | Geometry Kernel Boundary                                        |
| `CAD2SIM-REQ-011` | Meshing Architecture                                            |
| `CAD2SIM-REQ-012` | Mesh Quality Analyzer                                           |
| `CAD2SIM-REQ-013` | Engineering Model Generation                                    |
| `CAD2SIM-REQ-014` | Engineering Model Layer                                         |
| `CAD2SIM-REQ-015` | Determinism                                                     |
| `CAD2SIM-REQ-016` | Error Handling                                                  |
| `CAD2SIM-REQ-017` | Testing Architecture                                            |
| `CAD2SIM-REQ-018` | Testing Architecture + controlled regression models             |
| `CAD2SIM-REQ-019` | Performance Architecture                                        |
| `CAD2SIM-REQ-020` | This architecture document and associated project documentation |
| `CAD2SIM-NFR-001` | Layered component boundaries                                    |
| `CAD2SIM-NFR-002` | Testing Architecture                                            |
| `CAD2SIM-NFR-003` | Portability-oriented dependency boundaries                      |
| `CAD2SIM-NFR-004` | Reproducible testing and benchmark structure                    |
| `CAD2SIM-NFR-005` | Explicit Error Handling                                         |
| `CAD2SIM-NFR-006` | Dependency Direction and component boundaries                   |
| `CAD2SIM-NFR-007` | Explicit project scope                                          |
| `CAD2SIM-NFR-008` | Explicit commercial-kernel experience boundary                  |
| `CAD2SIM-NFR-009` | Design baseline versus implementation distinction               |
| `CAD2SIM-NFR-010` | Architecture-to-requirements traceability                       |

This table represents architectural intent only. It does not establish implementation or verification status.

---

## 18. Current Status

Architecture baseline defined.

Implementation has not yet started.

External CAD and meshing dependencies have not yet been selected, installed, or integrated.

No architecture component shall be marked implemented until source code and corresponding verification evidence exist.
