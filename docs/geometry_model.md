# CAD2Sim-Core Geometry Model

## 1. Purpose

This document defines the initial mathematical, geometric, and topological model of CAD2Sim-Core.

The model establishes the concepts and relationships that the C++ implementation shall provide for 3D computational-geometry and CAD preprocessing.

This document is a design baseline. The concepts described here shall not be considered implemented until corresponding source code and verification evidence exist.

---

## 2. Model Scope

The geometry model covers:

* 3D mathematical primitives
* coordinate systems and transformations
* geometric entities
* geometric properties
* B-Rep topology
* topology/geometry relationships
* numerical tolerances
* stable entity identification
* geometry validation concepts

The model does not attempt to reproduce the complete internal object model of any commercial CAD kernel.

---

## 3. Fundamental Mathematical Entities

### 3.1 Point3D

A `Point3D` represents a position in three-dimensional Euclidean space.

Conceptually:

```text
P = (x, y, z)
```

A point represents a location rather than a direction.

Required operations shall include, where applicable:

* point-to-point displacement
* translation by a vector
* distance calculation
* transformation by a coordinate transform

---

### 3.2 Vector3D

A `Vector3D` represents a direction and magnitude in three-dimensional space.

Conceptually:

```text
V = (x, y, z)
```

Core operations include:

* addition
* subtraction
* scalar multiplication
* dot product
* cross product
* magnitude
* normalization

The implementation shall define behavior for degenerate vectors where normalization is not mathematically valid.

---

### 3.3 CoordinateFrame

A `CoordinateFrame` represents a local coordinate system.

A frame shall conceptually contain:

* origin
* orthonormal basis directions
* relationship to a reference coordinate system

Coordinate frames are required for transformations and local geometric reasoning.

---

### 3.4 Transform3D

A `Transform3D` represents a transformation between coordinate systems or geometric positions.

The initial scope shall support:

* translation
* rotation
* composition
* inverse transformation
* point transformation
* vector transformation

Scaling may be supported where appropriate for preprocessing operations.

Rigid transformations shall preserve geometric distances and angles within numerical tolerance.

---

## 4. Geometric Entities

The geometry model distinguishes mathematical geometry from topological structure.

The primary geometric concepts are:

```text
Point
  │
  ├── Curve
  │
  └── Surface
         │
         ▼
       Solid
```

The exact representation may be provided by the selected CAD kernel through the kernel abstraction layer.

---

### 4.1 Curves

A curve represents a one-dimensional geometric entity embedded in 3D space.

Examples include:

* line
* circle
* arc
* spline

A curve may be associated with a topological edge but is not itself a topological edge.

---

### 4.2 Surfaces

A surface represents a two-dimensional geometric entity embedded in 3D space.

Examples include:

* plane
* cylinder
* sphere
* cone
* parametric surface

A surface may support geometric queries such as:

* point evaluation
* normal evaluation
* classification
* area-related calculations where defined

A surface may be associated with a topological face but is not itself a topological face.

---

### 4.3 Solid Geometry

A solid represents a bounded three-dimensional geometric region.

Relevant properties may include:

* volume
* centroid
* bounding box
* geometric validity

The solid-level representation shall remain distinct from its B-Rep topological structure.

---

## 5. Geometry Versus Topology

A fundamental design rule is:

```text
Geometry
    ≠
Topology
```

Geometry describes mathematical shape.

Topology describes connectivity and relationships.

For example:

```text
Face
 ├── Topological information
 │    ├── boundary wires
 │    ├── adjacent faces
 │    └── orientation
 │
 └── Geometric information
      └── underlying surface
```

Similarly:

```text
Edge
 ├── Topological information
 │    ├── connectivity
 │    ├── orientation
 │    └── adjacent faces
 │
 └── Geometric information
      └── underlying curve
```

This distinction is required to prevent higher-level preprocessing algorithms from confusing geometric classification with topological connectivity.

---

## 6. B-Rep Topology

The initial boundary-representation model follows the logical hierarchy:

```text
Solid
  │
  ▼
Shell
  │
  ▼
Face
  │
  ▼
Wire
  │
  ▼
Edge
  │
  ▼
Vertex
```

The hierarchy describes containment and connectivity relationships rather than a simple ownership tree.

A real B-Rep may contain shared topology and relationships that cannot be represented correctly as a simple tree.

---

### 6.1 Vertex

A `Vertex` represents a topological point.

It may reference an underlying geometric position.

Relevant information includes:

* geometric location
* connected edges
* stable identity where supported

---

### 6.2 Edge

An `Edge` represents a topological connection between vertices.

It may reference an underlying curve.

Relevant information includes:

* start/end vertices
* underlying curve
* orientation
* adjacent topology
* parameter range where available

---

### 6.3 Wire

A `Wire` represents an ordered collection of connected edges forming a topological boundary.

Typical uses include:

* outer face boundaries
* inner boundaries
* holes

The implementation shall preserve edge connectivity and orientation information where available.

---

### 6.4 Face

A `Face` represents a bounded topological region associated with an underlying surface.

Relevant information includes:

* underlying surface
* boundary wires
* orientation
* neighboring faces
* geometric properties

A face may contain inner wires representing holes or other internal boundaries.

---

### 6.5 Shell

A `Shell` represents a connected collection of faces forming part or all of a closed boundary.

Shells are important for distinguishing:

* closed solid boundaries
* open surfaces
* disconnected or problematic topology

---

### 6.6 Solid

A `Solid` represents a topological solid bounded by one or more shells.

The implementation shall distinguish the solid's topological representation from its geometric volume representation.

---

## 7. Topological Relationships

The model shall support relationships such as:

```text
Vertex
  ↕
Edge
  ↕
Face
  ↕
Shell
  ↕
Solid
```

and adjacency relationships such as:

```text
Face A ── Edge ── Face B
```

These relationships are important for:

* feature recognition
* connectivity analysis
* geometry validation
* engineering-region identification
* mesh preprocessing

---

## 8. Orientation

Orientation is part of the topological model.

Relevant orientation concepts include:

* edge direction
* wire traversal direction
* face orientation relative to its underlying surface
* shell orientation
* solid boundary orientation

The implementation shall not assume that geometric orientation can always be inferred solely from coordinates without considering B-Rep orientation information.

---

## 9. Geometric Properties

The geometry model shall provide access to properties required by preprocessing.

Initial properties include:

| Entity     | Representative properties                         |
| ---------- | ------------------------------------------------- |
| Point      | coordinates                                       |
| Curve      | length, endpoints, classification where available |
| Surface    | area, normal, classification where available      |
| Face       | area, normal, surface classification              |
| Solid      | volume, centroid, bounding box                    |
| Any entity | bounding information where applicable             |

Numerical results shall be evaluated using defined tolerances rather than inappropriate exact floating-point comparisons.

---

## 10. Geometric Classification

The model shall support classification of geometric entities.

Initial target classifications include:

```text
Surface
 ├── Plane
 ├── Cylinder
 ├── Sphere
 ├── Cone
 ├── Other analytic surface
 └── General / parametric surface
```

Curves may similarly be classified where supported.

Classification results shall distinguish:

* confirmed classification
* unsupported classification
* ambiguous classification

Ambiguity shall not silently become a confirmed feature.

---

## 11. Feature-Level Geometry

Feature recognition shall operate on combinations of geometry and topology.

Conceptually:

```text
Geometry
   +
Topology
   +
Geometric Properties
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

Examples of target features include:

* holes
* pockets
* planar regions
* cylindrical regions
* fillets
* chamfers

A feature is therefore treated as a higher-level engineering interpretation rather than as a primitive geometric entity.

---

## 12. Numerical Tolerances

CAD geometry frequently contains numerical approximations.

The implementation shall therefore avoid relying on exact floating-point equality for geometric decisions.

The model shall distinguish conceptually between:

* modeling tolerance
* comparison tolerance
* geometric classification tolerance
* validation tolerance

Tolerance values shall be explicit and configurable where appropriate.

Tolerance-sensitive algorithms shall document the assumptions under which their results are valid.

---

## 13. Entity Identity

Preprocessing operations may need stable references to geometric and topological entities.

The architecture shall provide a project-level strategy for identifying entities without exposing unnecessary vendor-specific identifiers.

Identity may be based on:

* deterministic traversal
* persistent application-level identifiers
* validated kernel references
* combinations of geometric/topological characteristics where appropriate

The selected strategy shall be defined during implementation.

The project shall not assume that a vendor-specific transient object address is a suitable persistent engineering identifier.

---

## 14. Geometry Validation

Geometry validation shall be treated as a dedicated preprocessing concern.

Validation may include:

* null or missing entities
* invalid topology
* disconnected topology
* open shells where closed geometry is required
* self-intersection or related geometric problems where detectable
* inconsistent orientation
* invalid numerical conditions

Validation results shall distinguish:

```text
Valid
Invalid
Warning
Unsupported
```

where such distinctions are meaningful.

Critical validation failures shall prevent generation of an invalid downstream engineering model.

---

## 15. Coordinate and Reference-System Strategy

The project shall distinguish between:

```text
Global Coordinate System
          │
          ▼
Local Coordinate Frame
          │
          ▼
Feature / Region Coordinates
```

Geometry-processing operations shall make coordinate-system assumptions explicit.

Transformations shall be composable and invertible for rigid transforms.

Round-trip transformations shall be verified within numerical tolerance where applicable.

---

## 16. Kernel Independence

The geometry model shall be expressed using project-level concepts wherever practical.

The application shall not require higher-level code to understand OpenCASCADE-specific classes.

Conceptually:

```text
CAD2Sim-Core Geometry Model
            │
            ▼
    Geometry Kernel API
            │
            ▼
     OpenCASCADE Adapter
```

The same conceptual boundary permits future integration of another geometry kernel without redesigning the complete application architecture.

This is an architectural extensibility mechanism and does not constitute professional experience with ACIS or Parasolid.

---

## 17. Serialization and Engineering References

Geometry and topology objects may need to be referenced by downstream engineering-model entities.

The serialized representation shall prefer stable application-level identifiers and metadata over direct serialization of external kernel objects.

A future engineering representation may therefore use concepts such as:

```text
Region
 ├── Region ID
 ├── Source geometry references
 ├── Feature references
 ├── Material reference
 └── Analysis metadata
```

The project shall not attempt to serialize opaque vendor-specific kernel objects as its primary engineering representation.

---

## 18. Geometry Model Verification

The geometry model shall eventually be verified through controlled reference cases.

Initial verification categories include:

### Mathematical Verification

* vector operations
* dot/cross products
* normalization
* distances
* transformations
* inverse transformations

### Geometric Verification

* analytical primitive properties
* surface classification
* bounding boxes
* areas
* volumes
* centroids

### Topological Verification

* entity counts
* connectivity
* adjacency
* orientation
* shell/solid relationships

### Robustness Verification

* invalid geometry
* degenerate entities
* unsupported entities
* tolerance-sensitive cases

### Regression Verification

* repeated processing of identical models
* deterministic topology summaries
* deterministic geometric-property output where applicable

No verification result is claimed at this stage.

---

## 19. Geometry Model to Requirements Traceability

| Requirement       | Geometry-model evidence                                 |
| ----------------- | ------------------------------------------------------- |
| `CAD2SIM-REQ-002` | Fundamental Mathematical Entities                       |
| `CAD2SIM-REQ-003` | Geometric Entities                                      |
| `CAD2SIM-REQ-005` | B-Rep Topology                                          |
| `CAD2SIM-REQ-006` | Geometry Validation                                     |
| `CAD2SIM-REQ-007` | Geometric Properties                                    |
| `CAD2SIM-REQ-008` | Feature-Level Geometry                                  |
| `CAD2SIM-REQ-009` | Coordinate and Reference-System Strategy                |
| `CAD2SIM-REQ-010` | Kernel Independence                                     |
| `CAD2SIM-REQ-013` | Serialization and Engineering References                |
| `CAD2SIM-REQ-015` | Regression Verification / determinism                   |
| `CAD2SIM-REQ-016` | Geometry Validation / Robustness                        |
| `CAD2SIM-REQ-017` | Geometry Model Verification                             |
| `CAD2SIM-REQ-018` | Regression Verification                                 |
| `CAD2SIM-REQ-020` | This document and associated verification documentation |

This table represents design intent only. It does not establish implementation or verification status.

---

## 20. Current Status

Geometry model baseline defined.

Implementation has not yet started.

No mathematical, geometric, topological, or kernel-integration capability described in this document shall be considered implemented until corresponding source code and verification evidence exist.
