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

### `simulate`

Runs the simulation for a given number of frames.

```python
sim.simulate(frames=120, dt=0.016)
```

**Parameters**:

- `frames` (`int`): Number of frames to simulate.
- `dt` (`float`): Time step for each frame.

### `reset`

Clears the simulation, removing all fabrics and colliders, and resetting the simulation parameters to their default
values.

```python
sim.reset()
```

### `soft_reset`

Resets all particles to their initial positions.

```python
sim.soft_reset()
```

### `save_scene`

Saves the defined scene to a JSON file. See [File Formats](../core/formats.md) for more information.

```python
sim.save_scene(path="path/to/scene.json", name="scene")
```

**Parameters**:

- `path` (`str`): Path to the JSON file where the scene will be saved.
- `name` (`str`): Name of the scene.

### `save_state`

Saves the current state of the simulation to a `.tissu` file. See [File Formats](../core/formats.md) for more
information.

```python
sim.save_state(path="path/to/state.tissu")
```

**Parameters**:

- `path` (`str`): Path to the `.tissu` file where the state will be saved.

### `save_snapshot`

Takes a geometry snapshot of the simulation and saves it to a `.obj` file.

```python
sim.save_snapshot(path="path/to/snapshot.obj", fabric_name="fabric")
```

**Parameters**:

- `path` (`str`): Path to the `.obj` file where the snapshot will be saved.
- `fabric_name` (`str`): Name of the fabric for which to save a snapshot.

### `load_scene`

Loads a scene from a JSON file with type `scene`.

```python
sim.load_scene(path="path/to/scene.json")
```

**Parameters**:

- `path` (`str`): Path to the JSON file containing the scene.

### `load_state`

Loads a simulation state from a `.tissu` file.

```python
sim.load_state(path="path/to/state.tissu")
```

**Parameters**:

- `path` (`str`): Path to the `.tissu` file containing the simulation state.

### `view`

Launches an interactive viewer to visualize the simulation. This requires the viewer to be built and available.

```python
sim.view(width=800, height=600, title="Viewer")
```

**Parameters**:

- `width` (`int`): Width of the viewer window.
- `height` (`int`): Height of the viewer window.
- `title` (`str`): Title of the viewer window.

### `bake_alembic`

Saves a simulation to an Alembic file.

```python
sim.bake_alembic(path="path/to/simulation.abc", start_frame=0, end_frame=120, fps=24)
```

**Parameters**:

- `path` (`str`): Path to the Alembic file where the simulation will be saved.
- `start_frame` (`int`): Starting frame of the simulation to bake.
- `end_frame` (`int`): Ending frame of the simulation to bake.
- `fps` (`float`): Frames per second for the baked simulation.