# Simulation

The `Simulation` class is the heart of Tissu's python API. It provides a way to use the core of Tissu easily, without
the need to write C++ code. This works as a wrapper around World and Solver classes, providing a simple manner to run
fabric simulations.

## Constructor

```python
from tissu import engine

sim = engine.Simulation(substeps=15, iterations=2, gravity=-9.81, thickness=0.05)
```

**Parameters:**

- `substeps` (`int`, default=`10`): Number of substeps per step. Increasing this value can improve stability, but also
  increases computation time.
- `iterations` (`int`, default=`2`): Number of iterations per substep. Increasing this value can improve solver
  convergence, but also increases computation time.
- `gravity` (`float`, default=`-9.81`): Gravity Y-component applied to the simulation.
- `thickness` (`float`, default=`0.02`): Thickness of the simulation, used to build the spatial hash.

## Properties

| Property      | Type      | Description                                                  |
|---------------|-----------|--------------------------------------------------------------|
| `substeps`    | `int`     | Number of substeps per step.                                 |
| `iterations`  | `int`     | Number of iterations per substep.                            |
| `gravity`     | `float`   | Gravity Y-component applied to the simulation.               |
| `thickness`   | `float`   | Thickness of the simulation, used to build the spatial hash. |
| `wind`        | `Vector3` | Wind vector applied to the simulation.                       |
| `air_density` | `float`   | Air density applied to the simulation.                       |
| `time`        | `float`   | Current simulation time.                                     |
| `frame`       | `int`     | Current simulation frame.                                    |
| `fabrics`     | `list`    | List of fabrics in the simulation.                           |

## Methods

### `create_grid`

Creates a fabric from a grid of particles.

```python
curtain = sim.create_grid(
    name="curtain",
    rows=100,
    cols=100,
    spacing=0.05,
    material="silk",
    rotation=[0.7071, 0.0, 0.0, 0.7071],
    translation=[-2.5, 4.0, -2.5]
)
```

**Parameters**:

- `name` (`str`): Name of the fabric.
- `rows` (`int`): Number of rows in the grid.
- `cols` (`int`): Number of columns in the grid.
- `spacing` (`float`): Spacing between particles in the grid.
- `material` (`str`, `dict`): Material of the fabric.
- `rotation` (`list`): Rotation of the fabric in quaternion format.
- `translation` (`list`): Translation of the fabric in world space.

### `create_from_obj`

Creates a fabric from an OBJ file. As real fabrics can't be modeled as a 3D mesh this function tends to be unstable.
2D modeling is marked as a future feature.

```python
dress = sim.create_from_obj(
    name="dress",
    path="path/to/dress.obj",
    material="cotton",
    rotation=[0.0, 0.0, 0.0, 0.0],
    translation=[0.0, 10.0, 0.0]
)
```

**Parameters**:

- `name` (`str`): Name of the fabric.
- `path` (`str`): Path to the OBJ file.
- `material` (`str`, `dict`): Material of the fabric.
- `rotation` (`list`): Rotation of the fabric in quaternion format.
- `translation` (`list`): Translation of the fabric in world space.

### `add_floor`

Adds a floor to the simulation.

```python
sim.add_floor(
    name="floor",
    height=1,
    friction=0.5,
)
```

**Parameters**:

- `name` (`str`): Name of the floor.
- `height` (`float`): Height of the floor in world space.
- `friction` (`float`): Friction coefficient of the floor.

### `add_sphere`

Adds a sphere to the simulation.

```python
sim.add_sphere(
    name="sphere",
    radius=1,
    center=[0, 0, 0],
    friction=0.5,
)
```

**Parameters**:

- `name` (`str`): Name of the sphere.
- `radius` (`float`): Radius of the sphere.
- `center` (`list`): Center of the sphere in world space.
- `friction` (`float`): Friction coefficient of the sphere.

### `add_capsule`

Adds a capsule to the simulation.

```python
sim.add_capsule(
    name="capsule",
    start=[0, 0, 0],
    end=[0, 1, 0],
    radius=0.5,
    friction=0.5,
)
```

**Parameters**:

- `name` (`str`): Name of the capsule.
- `start` (`list`): Start point of the capsule in world space.
- `end` (`list`): End point of the capsule in world space.
- `radius` (`float`): Radius of the capsule.
- `friction` (`float`): Friction coefficient of the capsule.

### `add_mesh`

Adds a mesh collider to the simulation.

Files Supported: OBJ

`````python
sim.add_mesh(
    name="mesh",
    path="path/to/mesh.obj",
    friction=0.5,
)
`````

**Parameters**:

- `name` (`str`): Name of the mesh.
- `path` (`str`): Path to the mesh file.
- `friction` (`float`): Friction coefficient of the mesh.

###  