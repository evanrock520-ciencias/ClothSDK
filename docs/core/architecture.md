# Architecture

## Structure

Knowing the state of memory and the flow of data in our programs is possibly the most important skill a programmer can have. This knowledge is the fundamental cornerstone for truly understanding the projects we work on. This document therefore aims to detail the architecture of **Tissu**, its layers of abstraction, and its real-time integration with Blender.

---

## 1. Philosophy of Abstraction Layers

Tissu's core philosophy is to offer high-quality simulations in a simple way. That's the reason behind the large number of abstraction layers that emerged during development. It would be cruel to force artists to work directly with particles and constraints, to recompile for every small change, and to deal with memory leaks, pointers, and templates. Nobody wants that (and probably nobody would use it).

Tissu has the advantage of being extensible and giving you the freedom to work however you prefer. It's precisely these extensions that enable deep integration with Blender or the automation of simulations via scripts. But to get there, it was necessary to build on a solid foundation.

![Structure](../assets/images/diagram_core_architecture.png)

---

## 2. Description of the Engine Layers

### A. Core (C++)

The foundation of everything lives in this layer, written in **C++**. It's the largest layer in terms of amount of code, and the densest, both conceptually and technically. It's divided into the following submodules:

```txt
core/include
├── data-structures
├── engine
├── io
├── math
├── physics
└── utils
```

- **`data-structures`:** Handles performance optimization through spatial data structures such as `BVH` (Bounding Volume Hierarchy) and `Spatial Hash`. They were designed to be model-agnostic and are widely used across the `physics` and `engine` modules.
- **`engine`:** Represents the scene's logic and state. It contains the main entities the user manipulates (`World`, `Cloth`, `ClothMesh`), abstracting the solver's raw data into friendly, descriptive objects.
- **`io`:** Manages internal and external data input/output: exporting animation caches (`AlembicExporter`, `OBJExporter`), loading 3D models (`OBJLoader`), JSON scene serialization (`SceneLoader`), and binary state serialization (`StateSerializer`).
- **`math`:** Tissu uses **Eigen** as its base math library. This module provides vector types, geometric transformations, and intersection utilities required by `physics` and `data-structures`.
- **`physics`:** The heart of XPBD (*Extended Position-Based Dynamics*). This is the most complex layer, containing the mass-point model (`Particle`), the execution engine (`Solver`), the constraint hierarchy (`Constraint`), forces (`Force`), and colliders (`Collider`).

---

### B. Bindings (`pybind11`)

An efficient core is the first building block of any simulation engine; however, the fastest engine is useless if it's complicated to use. Usability plays a role just as important as computational efficiency, and C++ isn't usually friendly for every workflow.

**Pybind11** gives us the ability to consume C++ code as native **Python** objects, providing a first layer of accessibility for building simulations without compiling and with a more agile syntax.

```txt
python
├── src
│   ├── bindings.cpp
│   └── bindings_headless.cpp
```

These files expose only the classes and functions necessary to configure and run the simulation, protecting internal structures that shouldn't be manipulated directly (such as `ConstraintGraph`).

During compilation, an extension binary is generated (`.so` on Linux/macOS or `.pyd` on Windows). This lets us import the native core directly into Python:

```python
from _cloth_sdk_core import World, Solver
```

---

### C. Python API (PyTissu)

Although the bindings solve the compilation-flow problem, they retain a low-level approach tied to C++ syntax. Scene configurations can require dozens of repetitive lines.

Using the `_cloth_sdk_core` binary module, **PyTissu** offers a truly *pythonic* interface. It works as a high-level *wrapper* based on the **Facade** design pattern, maintaining a clean interface, automatically synchronizing the physical world's components, and offering convenience methods.

Its structure is also fairly simple:

```txt
python
└── tissu
    ├── _cloth_sdk_core.pyi
    ├── engine.py
    ├── __init__.py
    ├── __main__.py

4 directories, 10 files
```

Below, the same simulation (a cloth falling onto a sphere) is written in both layers for comparison:

#### Via Bindings

```python
import numpy as np
from tissu._cloth_sdk_core import (
    World,
    Solver,
    ClothMaterial,
    Cloth,
    ClothMesh,
    GravityForce,
    PlaneCollider,
    AlembicExporter,
)

world = World()
solver = Solver()

material = ClothMaterial(density=0.1, structural=1e-9, shear=1e-9, bending=0.05)
curtain = Cloth("curtain", material)

mesh = ClothMesh()
mesh.init_grid(rows=80, cols=80, spacing=0.05, out_cloth=curtain, solver=solver)

gravity = GravityForce([0.0, -9.81, 0.0])
world.add_force(gravity)
world.add_cloth(curtain)

plane = PlaneCollider(origin=[0.0, 0.0, 0.0], normal=[0.0, 1.0, 0.0], friction=0.5)
world.add_collider(plane)

names = []
global_indices = []
particle_indices = []

for cloth in world.get_cloths():
    names.append(cloth.get_name())
    triangles = cloth.get_triangles()
    global_indices.append(np.array(triangles, dtype=np.int32))
    particle_indices.append(np.array(cloth.get_particle_indices(), dtype=np.int32))

particles = solver.get_particles()
global_positions = np.array(
    [p.get_position() for p in particles], dtype=np.float64
)

exporter = AlembicExporter()
success = exporter.open(
    path="data/animations/curtain.abc",
    names=names,
    global_positions=global_positions,
    global_indices=global_indices,
    particle_indices=particle_indices,
)

if not success:
    raise RuntimeError("Could not initialize the Alembic exporter")

total_frames = 100
dt = 1.0 / 60.0

for frame in range(total_frames):
    solver.update(world, dt)
    current_positions = np.array(
        [p.get_position() for p in solver.get_particles()], dtype=np.float64
    )
    exporter.write_frame(global_positions=current_positions, time=frame * dt)

exporter.close()
```

#### Via PyTissu

```python
from tissu import engine

sim = engine.Simulation()
cotton_curtain = sim.create_grid(
    name="cotton_curtain",
    rows=100,
    cols=100,
    spacing=0.05,
    material="silk",
    rotation=[0.7071, 0.0, 0.0, 0.7071],
    translation=[-2.5, 4.0, -2.5],
)

sim.add_floor("floor", height=-2.7)
sim.add_sphere("ball", center=[0.0, 0.0, 0.0], radius=1.0)

sim.bake_alembic(
    path="data/animations/falling_curtain.abc",
    start_frame=1,
    end_frame=120,
)
```

---

### D. Tissu CLI

Building on the `IO` and `PyTissu` modules, the command-line interface emerges. It works as an extension that processes scene and state serialization formats (`.json` and `.tissu`). It allows running and exporting complete simulations directly from the terminal using simple commands, with no need to write code.

---

### E. Tissu Blender

`Tissu Blender` is the most complex layer after the core in terms of software design. It connects the C++ engine to Blender in real time, integrating directly into the 3D artist's workflow.

This layer's internal structure is organized inside the `blender/` folder and is divided into clearly delimited responsibilities:

```txt
blender/
├── __init__.py          # Entry point and dynamic core injection
├── libs/                # Compiled C++ binaries (_cloth_sdk_core)
├── panels/               # User interface and gizmos (N-Panel UI)
├── pattern/              # Custom Blender properties and operators
└── simulation/
    ├── bridge.py         # Adapter from the Python API to the C++ world
    └── session.py        # State management, lifecycle, and coordinate transformations
```

- **`pattern/properties.py`:** By default, Blender doesn't distinguish which meshes should behave as cloth or as obstacles. This module extends Blender's data model by registering Custom Properties on scene objects and on the scene itself: `tissu_is_fabric` marks an object as cloth, `tissu_is_collider` marks it as a collider, and there are additional parameters for friction and volume preservation. Vertex groups are also inspected dynamically; a group named `"Tissu_Pins"` is automatically mapped as rigid pinning constraints (`PinConstraint`) in the C++ engine.
- **`simulation/session.py`:** Blender uses the $Z$ axis as up ($Z$-Up); Tissu's core uses the $Y$ axis as up ($Y$-Up). `session.py` resolves this discrepancy with a vector conversion in NumPy: it swaps the axes and multiplies the positions by the object's inverse matrix (`matrix_world.inverted()`). This makes it possible to compute particles in world coordinates in C++ and write the result into the Blender object's local space, regardless of its position or rotation in the scene.
- **Real-time data transfer:** updating thousands of vertices per frame can't rely on vertex-by-vertex Python loops. Blender's vectorized calls `foreach_get` and `foreach_set` are used instead, reading and writing coordinates directly to contiguous memory blocks, achieving sub-millisecond transfer.
- **Depsgraph integration:** the flow is orchestrated through handlers that listen for changes in the timeline (*Depsgraph*). When the simulation starts at frame 1, `capture_fabric_rest_positions` captures a clean copy of the original geometry. On each subsequent frame, `sim.step` computes the new state, updates the on-screen mesh, and stores the positions in a session cache (`_point_cache`), which enables instant scrubbing backward and forward over frames that have already been simulated.

---

These are all of Tissu's layers for now, though it shouldn't be too complicated to add integrations with other DCCs or for other uses, thanks to the engine's architectural structure.

The entire Tissu project is built around the C++ core. As Tissu's philosophy is to provide a high-performance simulation
engine alongside an easy-to-use interface, connecting C++ and Python is a key aspect of the project. This would be
almost impossible to achieve without Pybind11, which allows us to expose C++ classes and functions to Python in a very
simple way. This acts as a bridge between all the artist tools built around the core and the core itself.

## Simulation Pipeline

## Source Code Structure

## Architectural Patterns
