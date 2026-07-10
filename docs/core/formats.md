# Format Files

Currently, Tissu supports two own formats for saving and loading simulation data. The first one is a JSON format, which
allows us to work with scenes, materials and physics. The second one is a binary format which allows us to save and load
simulation states, i.e have the ability to save and load the simulation at any point in time. In this section we will
explore both of them.

## Scenes

While Tissu workflow is designed to work with the Python API, it is also possible to use Tissu with scene files.
The scene files are written in JSON format and can be used to define the simulation parameters, fabrics and colliders
without the need to write code. This is specially useful when
using [Tissu CLI](https://github.com/evanrock520-ciencias/tissu-cli).

### Structure

### `metadata`

Every scene file must contain the metadata header, which contains 3 fields: `version`, `type` and `name`.
Let's notice the following:

- The current version of this format is `2.0`
- There are three types of scene files: `scene`, `material` and `physics`.
- There is no backwards compatibility between versions.

This is an example of a valid metadata header:

````json
{
  "version": "2.0",
  "type": "scene",
  // or "material" or "physics"
  "name": "test"
}
````

### `physics`

The physics section contains the simulation parameters, such as substeps, iterations, gravity, thickness, wind and air
density. This section accept a set of parameters or a physics configuration file path.

This is an example of a valid physics section:

````json
{
  "physics": {
    "substeps": 10,
    "iterations": 2,
    "gravity": [
      0.0,
      -9.81,
      0.0
    ],
    "collision": {
      "thickness": 0.05
    },
    "environment": {
      "wind": [
        0.0,
        0.0,
        0.0
      ],
      "air_density": 0.1
    }
  }
}
````

### `fabrics`

This section is probably the most important one, as it contains the fabrics that will be simulated.
It is important to remember that there are two ways to define a fabric: using a grid of particles or using a mesh. For
the former, we can define the number of rows and columns and the spacing between particles. For the latter, we can
define the path to the mesh file. They also share some common parameters, such as the material, rotation and
translation.

This is an example for a fabric defined using a grid of particles:

````json
{
  "fabrics": [
    {
      "name": "curtain",
      "type": "grid",
      "rows": 100,
      "cols": 100,
      "spacing": 0.05,
      "material": "silk",
      "rotation": [
        0.7071,
        0.0,
        0.0,
        0.7071
      ],
      "translation": [
        -2.5,
        4.0,
        -2.5
      ],
      "pins": {
        "mode": "top_corners",
        "compliance": 0.0
      }
    }
  ]
}
````

This is an example for a fabric defined using a mesh:

````json
{
  "fabrics": [
    {
      "name": "dress",
      "type": "mesh",
      "path": "path/to/dress.obj",
      "material": "cotton",
      "rotation": [
        0.0,
        0.0,
        0.0,
        0.0
      ],
      "translation": [
        0.0,
        10.0,
        0.0
      ]
    }
  ]
}
````

### `colliders`

The last section is the colliders section. There are four types of colliders: plane, sphere, capsule and mesh.
Basically, they are all defined by different parameters as they just share the friction parameter, so let's see some
examples.

This is an example for a plane collider:

````json
{
  "colliders": [
    {
      "name": "floor",
      "type": "plane",
      "origin": [
        0.0,
        0.0,
        0.0
      ],
      "normal": [
        0.0,
        1.0,
        0.0
      ],
      "friction": 0.5
    }
  ]
}
````

This is an example for a sphere collider:

````json
{
  "colliders": [
    {
      "name": "sphere",
      "type": "sphere",
      "radius": 1.0,
      "center": [
        0.0,
        0.0,
        0.0
      ],
      "friction": 0.5
    }
  ]
}
````

This is an example for a capsule collider:

````json
{
  "colliders": [
    {
      "name": "capsule",
      "type": "capsule",
      "start": [
        0.0,
        0.0,
        0.0
      ],
      "end": [
        0.0,
        1.0,
        0.0
      ],
      "radius": 0.5,
      "friction": 0.5
    }
  ]
}
````

This is an example for a mesh collider:

````json
{
  "colliders": [
    {
      "name": "mesh",
      "type": "mesh",
      "path": "path/to/mesh.obj",
      "friction": 0.5
    }
  ]
}
````

At the end, a valid scene file should look like this:

````json
{
  "version": "2.0",
  "type": "scene",
  "name": "test",
  "physics": {
    "substeps": 10,
    "iterations": 2,
    "gravity": [
      0.0,
      -9.81,
      0.0
    ],
    "collision": {
      "thickness": 0.05
    },
    "environment": {
      "wind": [
        0.0,
        0.0,
        0.0
      ],
      "air_density": 0.1
    }
  },
  "fabrics": [
    {
      "name": "curtain",
      "type": "grid",
      "rows": 80,
      "cols": 80,
      "spacing": 0.05,
      "material": {
        "density": 0.1,
        "compliance": {
          "structural": 1e-09,
          "shear": 1e-08,
          "bending": 0.1
        }
      }
    }
  ],
  "colliders": [
    {
      "type": "plane",
      "origin": [
        0.0,
        0.0,
        0.0
      ],
      "normal": [
        0.0,
        1.0,
        0.0
      ],
      "friction": 0.5
    }
  ]
}
````

At the end, we don't need to remember all the parameters, as we can use the Tissu CLI or use the Python API to generate
the scene file.

### Physics Files

The physics files works in a similar way as the scene files and basically they contain a little part of what a scene
files does. To define a physics file we need to define the metadata header and the physics section. This is an example
of a valid physics file:

````json
{
  "version": "2.0",
  "type": "physics",
  "name": "bake",
  "substeps": 40,
  "iterations": 8,
  "gravity": [
    0.0,
    -9.81,
    0.0
  ],
  "collision": {
    "thickness": 0.02
  },
  "environment": {
    "wind": [
      0.0,
      0.0,
      0.0
    ],
    "air_density": 0.1
  }
}
````

### Material Files

The material files stores the material properties of a fabric, i.e density, structural, shear and bending compliance. To
define a material file we need to define the metadata header and the material section. This is an example of a valid
material file:

````json
{
  "version": "2.0",
  "type": "material",
  "name": "silk",
  "density": 0.1,
  "compliance": {
    "structural": 1e-9,
    "shear": 1e-8,
    "bending": 0.1
  }
}
````    

## Checkpoints

To enable the ability to save and load the simulation at any point in time, Tissu provides a binary format for saving
and loading states with extension `.tissu`. This binary format is designed to be fast, efficient and easy to use.