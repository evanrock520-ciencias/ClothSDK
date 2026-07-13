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
Let's explore the structure of this format.

### Metadata Header

The metadata header is the first part of the `.tissu` file. By following the Unix philosphy the file format header
contains a magic bytes sequence that identifies the file format, a version number, flags, the current frame, the current
timestamp and the particle count. It also contains a data validation checksum with CRC32, which I will explain later.

| Offset | Size | Type     | Description                      |
|--------|------|----------|----------------------------------|
| 0      | 6    | char[6]  | Magic bytes sequence: "TISSU\0"  |
| 6      | 1    | uint8_t  | Version number                   |
| 7      | 1    | uint8_t  | Flags                            |
| 8      | 4    | uint32_t | Current frame                    |
| 12     | 8    | double   | Current timestamp                |
| 20     | 4    | uint32_t | Particle count                   |
| 24     | 4    | uint32_t | Data validation checksum (CRC32) |
| 28     | 4    | uint32_t | Padding                          |    

### Data Section

The data section is the second part of format file and possibly the most important one. As we want to know the exact
state of the simulation at this checkpoint, we need to store every single thing that could change during the simulation.

That's the reason why we need to serialize the world parameters, even though they are defined always via scene files or
scripts. They are stored in this order:

| Offset | Size | Type      | Description    |
|--------|------|-----------|----------------|
| 32     | 24   | double[3] | Gravity vector |
| 56     | 24   | double[3] | Wind           |
| 80     | 8    | double    | Air density    |
| 88     | 8    | double    | Thickness      |

Then we need to serialize the colliders, as they became kinematic objects we need to store their position and rotation
since version 2.0. At this point, the offsets are variable because of the number of colliders and their types. For every
collider we store the following data no matter the type.

### `colliders`

| Size | Type      | Description       |
|------|-----------|-------------------|
| 24   | double[3] | Position          |
| 32   | double[4] | Rotation          |
| 24   | double[3] | Previous Position |
| 32   | double[4] | Previous Rotation |
| 4    | uint32_t  | Name Length       |
| L    | char[L]   | Name              |
| 1    | uint8_t   | Type              |
| 8    | double    | Friction          |

As every collider has a different set of parameters, we need to store them in a different way based on their `Type`
field.

#### `Type 0: Sphere`

| Size | Type      | Description |
|------|-----------|-------------|
| 24   | double[3] | Center      |
| 8    | double    | Radius      |

#### `Type 1: Plane`

| Size | Type      | Description |
|------|-----------|-------------|
| 24   | double[3] | Origin      |
| 24   | double[3] | Normal      |

#### `Type 2: Capsule`

| Size | Type      | Description |
|------|-----------|-------------|
| 24   | double[3] | Start Point |
| 24   | double[3] | End Point   |
| 8    | double    | Radius      |

#### `Type 3: Mesh`

Mesh colliders do not require additional specific parameters stored in the `.tissu` file, as their state is fully
defined by the position and rotation stored in the general collider data above.

### Particles

After the colliders, the particles of the simulation are serialized sequentially. The total number of particles is
defined in the `Particle count` field of the metadata header.

| Size | Type      | Description  |
|------|-----------|--------------|
| 24   | double[3] | Position     |
| 24   | double[3] | Old Position |
| 8    | double    | Inverse Mass |

### Constraints

Finally, the state of the solver's constraints are serialized. First, a `uint32_t` is
stored specifying the total number of constraints, followed by each constraint's `lambda` value.

| Size | Type     | Description       |
|------|----------|-------------------|
| 4    | uint32_t | Constraints count |

Then, for each constraint:

| Size | Type   | Description |
|------|--------|-------------|
| 8    | double | Lambda      |

<!-- Explain CRC32 -->

### CRC32 Checksum

CRC32 is a widely used error-detecting code that is used to detect accidental changes to raw data. For instance, it is
used in formats like PNG, ZIP and PHP.

CRC32 is based on the Galois field $GF(2)$ i.e a field with two elements, 0 and 1. Here, the basic operations such as
addition and multiplication are defined modulo 2. That means that addition is equivalent to the XOR operation and
multiplication is equivalent to the AND operation. In this field, we can represent polynomials as binary numbers, where
each bit represents a coefficient of the polynomial. A commonly used polynomial for CRC32 is `0xEDB88320`, which is the
inverse of the polynomial `0x04C11DB7`, which can be represented as:

$$f(x) = x^{32} + x^{26} + x^{23} + x^{22} + x^{16} + x^{12} + x^{11} + x^{10} + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1$$

The `StateSerializer` class in Tissu uses the CRC32 algorithm to compute a checksum for the data section of the `.tissu`
file. It's defined by two functions: `initCrcTable` and `computeCRC32`.

````c++
static constexpr uint32_t CRC32_POLY = 0xEDB88320;
````

```c++
void StateSerializer::initCrcTable() {
    if (crcTableReady)
        return;
    for (uint32_t idx = 0; idx < 256; idx++) {
        uint32_t crc = idx;
        for (int jdx = 0; jdx < 8; jdx++) {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC32_POLY;
            else
                crc >>= 1;
        }
        buildCrcTable[idx] = crc;
    }
    crcTableReady = true;
}
```

First, this function initializes a lookup table for the CRC32 algorithm. It iterates over all possible byte values (
0-255) and precomputes the value for each byte. The polynomial used for the CRC32 calculation is defined by the
constant `CRC32_POLY`, which is set to `0xEDB88320`. For each byte value, the function performs 8 iterations of bitwise
operations to compute the CRC32 value, if the least significant bit of the current CRC value is set, it right shifts the
CRC value by 1 and XORs it with the polynomial. Otherwise, it just right shifts the CRC value by 1. The computed CRC32
value for each byte is stored in the `buildCrcTable` array.

The second function computes the CRC32 checksum for a given data buffer.

```c++
uint32_t StateSerializer::computeCRC32(const uint8_t* data, size_t length) {
    initCrcTable();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t idx = 0; idx < length; idx++)
        crc = (crc >> 8) ^ buildCrcTable[(crc ^ data[idx]) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}
```

We use the first function to initialize the table if it hasn't been initialized yet. Then, we initialize the CRC value
to `0xFFFFFFFF`. For each byte in the data buffer, we update the CRC value by right shifting it by 8 bits and XORing it
with the precomputed value from the lookup table. The index for the lookup table is calculated by XORing the current CRC
value with the current byte and masking it with `0xFF` to ensure it is within the range of 0-255. Finally, we return the
final CRC value by XORing it with `0xFFFFFFFF`.

This is not an infallible method to detect errors, is not even close to be a cryptographic method, but it is a good way
to ensure that the data has not been corrupted during some point in time. If the calculated checksum does not match the
stored checksum, it indicates that the data has been altered or corrupted before it used in the simulation. 

