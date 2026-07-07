# Architecture

## Project Structure

Below is the directory layout of Tissu, showing the main folders of the codebase:

```text
.
├── benchmarks              # Performance benchmarks
├── core                    # C++ Core SDK source and headers
│   ├── include             # Public API headers
│   │   ├── data-structures # e.g. Spatial Hash
│   │   ├── engine          # World, Cloth representation
│   │   ├── io              # Importers and exporters (OBJ, Alembic, JSON)
│   │   ├── math            # Math utilities
│   │   ├── physics         # Solvers, Constraints, Particles, Forces
│   │   └── utils           # Logging utilities
│   └── src                 # C++ Implementation source files
├── data                    # Assets (configs, scene files, models, animations, states)
├── docs                    # Documentation (markdown files, diagrams, videos)
├── examples                # Code examples using Tissu
├── python                  # Python bindings and API packaging
│   ├── src                 # Pybind11 wrapper sources
│   └── tissu               # Pure python API
├── tests                   # Unit and integration test suites
└── viewer                  # Real-time OpenGL & ImGui standalone viewer application
```
