# Code Style Guide

## Style Guide

There are some style guide rules that we follow in Tissu.

### C++ Style Guide

### `Naming conventions`

| Type      | Convention                          | Example               |
|-----------|-------------------------------------|-----------------------|
| Class     | PascalCase                          | `DistanceConstraint`  |
| Struct    | PascalCase                          | `Triangle`            |
| Function  | camelCase                           | `solveSelfCollisions` |
| Variable  | camelCase                           | `deltaTime`           |
| Parameter | camelCase                           | `maxIterations`       |
| Property  | Starts with `m_` and uses camelCase | `m_compliance`        |

### `Formatting`

- The formatting of the C++ code is done using clang-format.
- It uses a LLVM-based style with some customizations.
- The configuration file is located in the root directory of the project and is named `.clang-format`.

Key formatting rules include:

- Strict indentation of **4 spaces** (no tabs).
- Pointers and references align to the left (e.g., `const Eigen::Vector3d& pos`).
- Opening braces are attached to the same line (`Attach` style).

### `Others`

- The C++ version used is C++17.
- Every `.cpp` file should include its corresponding `.hpp` file.
- At the beginning of each header file, we should include `#pragma once`.
- For documentation, we use Doxygen-style block comments (`/** ... */`) for classes and methods.

### Python Style Guide

### `Naming conventions`

| Type             | Convention                          | Example           |
|------------------|-------------------------------------|-------------------|
| Class            | PascalCase                          | `Simulation`      |
| Function         | snake_case                          | `bake_alembic`    |
| Variable         | snake_case                          | `fabrics`         |
| Private Property | Starts with `_` and uses snake_case | `_gravity_vector` |

### `Formatting`

The formatting and linting of the Python code is done using `ruff` (configured in `pyproject.toml` and
`.pre-commit-config.yaml`).

Key formatting rules include:

- Indentation of **4 spaces**.
- Double quotes (`"..."`) for strings.
- Line length limit of **120 characters**.
- The target version is **Python 3.12** or higher.
- Strict use of **Type Hinting** (e.g., `substeps: int = 10`), including `from __future__ import annotations` at the top
  of files for forward references.

## Documentation and Assets

All the documentation about Tissu is located in the `docs/` directory. It is organized by subjects, as it seen in the
follow tree:

```txt
├───assets          // Contains all the assets used in the documentation
│   ├───images      // Contains all the images used in the documentation
│   └───videos      // Contains all the videos used in the documentation
│       ├───blender // Used to save Blender renders.
│       ├───manim   // Used to save math animations.
│       └───viewer  // Used to save viewer related videos.
├───blender         // Contains all the Blender add-on related documentation
├───cli             // Contains all the Tissu CLI related documentation
├───core            // Contains all the core related documentation
└───python          // Contains all the Python API related documentation
```

Each new markdown file should be added to the corresponding directory. If a new subject is needed, a new directory
should be created.
The naming convention for the markdown files is to use snake_case. For example, `my_new_documentation.md`.

Each asset used in the documentation should be added to the `assets/` directory. Depending on the type, the asset should
be added to `images/` or `videos/`.
To name the asset we follow these conventions:

- We use snake_case for the asset name.
- We follow the structure `<prefix>_<module>_<description>.<extension>`.

The `<prefix>` value is given by the use of the asset. It can be one of the following:

- `diagram`: Used for technical diagrams.
- `demo`: Used for demo videos.
- `math`: Used for math animations and related assets.
- `ui`: Used for UI related images or videos.

The `<module>` value is given by the module of Tissu that the asset is related to. It can be one of the following:

- `core`: Used for core related assets.
- `viewer`: Used for viewer related assets.
- `python`: Used for Python API related assets.
- `blender`: Used for Blender add-on related assets.
