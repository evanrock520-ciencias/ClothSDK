# Tissu CLI

![Init](https://vhs.charm.sh/vhs-3rvRe8ZU98Pk7flGTkoN77.gif)

Another way to interact with Tissu is through his command line interface. By reusing the Python API, but especially the
scene format (see [File formats](../core/formats.md)), this CLI allows you to do everything you can do with the
Python API, but without writing any code. [Tissu CLI](https://github.com/evanrock520-ciencias/tissu-cli) is available from
PyPI, so you can install it via pip.

---

## Installation

```bash
pip install tissu-cli
```

Or from source:

```bash
git clone https://github.com/evanrock520-ciencias/tissu-cli.git
cd tissu-cli
pip install .
```

Enable shell completion (optional):

```bash
tissu-cli --install-completion
```

---

## Quickstart

Create a scene from scratch, simulate, and view the result:

```bash
# 1. Initialize a new scene
tissu-cli init demo

# 2. Add a fabric grid pinned at the top corners
tissu-cli add-fabric demo cloth --rows 80 --cols 80 --pin-mode top_corners

# 3. Add a ground plane collider
tissu-cli add-collider demo --plane

# 4. Bake to Alembic
tissu-cli bake demo -o demo.abc --end 120

# 5. Open the 3D viewer
tissu-cli view demo
```

---

## Command Reference

### Scene Management

#### `init`

Initialize a new scene directory with a blank scene file.

```bash
tissu-cli init <dir> [OPTIONS]
```

![Init](https://vhs.charm.sh/vhs-6rEAgJylh8isDap7DABpQf.gif)

| Option          | Default       | Description               |
|-----------------|---------------|---------------------------|
| `--substeps`    | `10`          | Solver substeps           |
| `--iterations`  | `2`           | Solver iterations         |
| `--gravity`     | `0,-9.81,0`   | Gravity vector (x,y,z)    |
| `--thickness`   | `0.05`        | Collision thickness       |
| `--wind`        | `0,0,0`       | Wind vector (x,y,z)       |
| `--air-density` | `0.1`         | Air density               |

#### `info`

Show information about a scene (`.json`) or state (`.tissu`) file.

```bash
tissu-cli info <dir>
```

Supports `--json` for machine-readable output.

#### `list-files`

List available files (scenes, materials, physics, states) in a directory.

```bash
tissu-cli list-files <dir> [OPTIONS]
```

| Option                | Description                        |
|-----------------------|------------------------------------|
| `-m`, `--materials`   | Show materials                     |
| `-p`, `--physics`     | Show physics                       |
| `-s`, `--scenes`      | Show scenes                        |
| `-e`, `--states`      | Show states                        |
| `-r`, `--recursive`   | Search recursively                 |

#### `physics`

Load a physics config file into a scene.

```bash
tissu-cli physics <dir> <physics_path>
```

#### `save-physics`

Extract and save the physics config from a scene.

```bash
tissu-cli save-physics <dir> <out_path> [OPTIONS]
```

| Option   | Default    | Description           |
|----------|------------|-----------------------|
| `--name` | `physics`  | Physics config name   |

#### `save-state`

Run a simulation and save the solver state to a `.tissu` file.

```bash
tissu-cli save-state <dir> <out_path> <frame>
```

#### `inspect-physics`

Display the contents of a physics preset file.

```bash
tissu-cli inspect-physics <path>
```

Supports `--json` for machine-readable output.

#### `inspect-material`

Display the contents of a material file.

```bash
tissu-cli inspect-material <path>
```

Supports `--json` for machine-readable output.

---

### Fabric

#### `add-fabric`

Add a fabric to a scene directory.

```bash
tissu-cli add-fabric <dir> <name> [OPTIONS]
```

![Fabric](https://vhs.charm.sh/vhs-6i9be2HnRuXMde5gaKhEOs.gif)

| Option                | Default  | Description                            |
|-----------------------|----------|----------------------------------------|
| `--rows`, `-r`        | —        | Grid rows (required for grid)          |
| `--cols`, `-c`        | —        | Grid cols (required for grid)          |
| `--spacing`           | `0.05`   | Distance between particles             |
| `--path`, `-p`        | —        | Path to OBJ mesh (alternative to grid) |
| `--density`           | `0.1`    | Material density                       |
| `--structural`        | `1e-9`   | Structural compliance                  |
| `--shear`             | `1e-8`   | Shear compliance                       |
| `--bending`           | `0.1`    | Bending compliance                     |
| `--pin-mode`          | —        | Pin mode: `top_corners`, `by_height`   |
| `--pin-compliance`    | `1e-9`   | Pin compliance                         |
| `--pin-threshold`     | `0.01`   | Pin threshold                          |

#### `pin`

Add pins to an existing fabric.

```bash
tissu-cli pin <dir> <fabric> <mode> [OPTIONS]
```

![Pin](https://vhs.charm.sh/vhs-7LKYGnhFP3dL44HyPuoNAb.gif)

| Option           | Default | Description                       |
|------------------|---------|-----------------------------------|
| `--compliance`   | `1e-9`  | Pin constraint compliance         |
| `--threshold`    | `0.01`  | Pin constraint threshold          |

`mode` must be `top_corners` or `by_height`.

#### `unpin`

Remove all pins from a fabric.

```bash
tissu-cli unpin <dir> <fabric>
```

#### `apply-material`

Apply a material file or override material properties on a fabric.

```bash
tissu-cli apply-material <dir> <fabric> [OPTIONS]
```

![Material](https://vhs.charm.sh/vhs-46kWJZwjrSi8aTIOkbfCB9.gif)

| Option              | Description                        |
|---------------------|------------------------------------|
| `-p`, `--path`      | Path to material JSON file         |
| `-d`, `--density`   | Override material density          |
| `-s`, `--structural`| Override structural compliance     |
| `-c`, `--shear`     | Override shear compliance          |
| `-b`, `--bending`   | Override bending compliance        |

At least `--path` or one property override is required.

#### `create-material`

Create and save a material JSON file.

```bash
tissu-cli create-material <path> <name> [OPTIONS]
```

| Option         | Default  | Description          |
|----------------|----------|----------------------|
| `--density`    | `0.1`    | Material density     |
| `--structural` | `1e-9`   | Structural compliance|
| `--shear`      | `1e-8`   | Shear compliance     |
| `--bending`    | `0.01`   | Bending compliance   |

#### `extract-material`

Extract the material from a fabric into its own JSON file.

```bash
tissu-cli extract-material <dir> <fabric> <out_path>
```

#### `remove-fabric`

Remove a fabric from a scene.

```bash
tissu-cli remove-fabric <dir> <fabric>
```

---

### Colliders

#### `add-collider`

Add a collider to a scene directory. One of `--plane`, `--sphere`, `--capsule`, or `--mesh` is required.

```bash
tissu-cli add-collider <dir> [OPTIONS]
```

![Collider](https://vhs.charm.sh/vhs-1HXp3lOfo5sRDcBs8o83Ln.gif)

| Option       | Default       | Description                         |
|--------------|---------------|-------------------------------------|
| `--name`, `-n`| `""`         | Collider name                       |
| `--plane`    | —             | Add a plane collider                |
| `--origin`   | `0,0,0`       | Plane origin (x,y,z)                |
| `--normal`   | `0,1,0`       | Plane normal (x,y,z)                |
| `--sphere`   | —             | Add a sphere collider               |
| `--center`   | `0,0,0`       | Sphere center (x,y,z)               |
| `--capsule`  | —             | Add a capsule collider              |
| `--start`    | `0,0,0`       | Capsule start (x,y,z)               |
| `--end`      | `0,1,0`       | Capsule end (x,y,z)                 |
| `--radius`   | `0.5`         | Sphere/capsule radius               |
| `--friction` | `0.5`         | Friction coefficient                |
| `--mesh`     | —             | Add a mesh collider                 |
| `--mesh-path`| —             | Path to OBJ mesh (required for mesh)|

#### `remove-collider`

Remove a collider from a scene.

```bash
tissu-cli remove-collider <dir> <collider>
```

---

### Simulation

#### `bake`

Bake a cloth simulation and export it to Alembic (`.abc`).

```bash
tissu-cli bake <dir> [OPTIONS]
```

![Bake](https://vhs.charm.sh/vhs-GU7QUZRdSiUNChffg9v5w.gif)

| Option          | Default       | Description          |
|-----------------|---------------|----------------------|
| `--out`, `-o`   | `output.abc`  | Output Alembic path  |
| `--start`       | `1`           | Start frame          |
| `--end`         | `120`         | End frame            |
| `--fps`         | `24.0`        | Frames per second    |
| `--state`, `-s` | —             | Load initial state   |

#### `view`

View a cloth simulation in a 3D viewer.

```bash
tissu-cli view <dir> [--state, -s <path>]
```

#### `snapshots`

Export OBJ mesh snapshots for each frame of a simulation.

```bash
tissu-cli snapshots <dir> <out_dir> <cloth> [OPTIONS]
```

| Option          | Default       | Description          |
|-----------------|---------------|----------------------|
| `--start`       | `1`           | Start frame          |
| `--end`         | `120`         | End frame            |
| `--state`, `-s` | —             | Load initial state   |

---

### Visualization

#### `plot`

Visualize a simulation frame in a 3D matplotlib plot.

```bash
tissu-cli plot <dir> <frame> [OPTIONS]
```

| Option     | Default | Description          |
|------------|---------|----------------------|
| `--fabric` | —       | Fabric name to plot  |

#### `plot-gif`

Generate an animated GIF preview of a simulation.

```bash
tissu-cli plot-gif <dir> <out_path> [OPTIONS]
```

| Option          | Default | Description            |
|-----------------|---------|------------------------|
| `--fabric`      | —       | Fabric name to render  |
| `--start`, `-s` | `1`     | Start frame            |
| `--end`, `-e`   | `120`   | End frame              |
| `--fps`         | `30.0`  | Frames per second      |

Output is written to `<dir>/exports/preview/<out_path>`.

#### `plot-energy`

Simulate and plot energy over time.

```bash
tissu-cli plot-energy <dir> <frame>
```

---

## Examples

### Curtain scene (using sample data)

Sample scenes and assets are included under `data/`:

```bash
tissu-cli info data/scenes/curtain
tissu-cli bake data/scenes/curtain -o curtain.abc
tissu-cli snapshots data/scenes/curtain ./frames curtain
```

### Custom scene with grid fabric and pinned curtain

```bash
tissu-cli init my_scene
tissu-cli add-fabric my_scene curtain --rows 80 --cols 80 --pin-mode top_corners
tissu-cli add-collider my_scene --plane
tissu-cli add-collider my_scene --sphere --center 0,-0.5,0 --radius 0.3
tissu-cli bake my_scene -o my_scene.abc --end 120
tissu-cli view my_scene
```

### Fabric from mesh + material override

```bash
tissu-cli init garment
tissu-cli add-fabric garment dress --path dress.obj --density 0.05
tissu-cli add-collider garment --mesh --mesh-path mannequin.obj
tissu-cli apply-material garment dress --density 0.08 --structural 1e-10
tissu-cli bake garment -o garment.abc
```