# Build and Develops

By starting from the source code, these are the prerequisites to build and develop the project.

## Prerequisites

### Toolchain

- **CMake** >= 3.20
- **Python** >= 3.12
- **C++ Compiler**:
    - **Linux**: GCC >= 9 or Clang >= 10
    - **macOS**: Apple Clang >= 12
    - **Windows**: MSVC >= 2019

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y build-essential cmake python3 python3-pip git git
```

### Linux (Fedora)

```bash
sudo dnf install -y gcc gcc-c++ make cmake python3 python3-pip git
```

### macOS

```bash
xcode-select --install
brew install cmake python git libomp
```

### Windows

```powershell
# Install Visual Studio 2019 or later with C++ development tools
# Install Cmake, Python and Git from their official websites or using scoop or chocolatey
# scoop install cmake python git
# choco install cmake python git
```

## Building the Project

The first step is to clone the repository:

```bash
git clone https://github.com/evanrock520-ciencias/Tissu.git
cd Tissu
```

To build the project, I recommend using the build script.

```bash
python scripts/build.py 
```

### Build Script Flags (`build.py`)

- `--no-compile`: Skips the compilation step.
- `--no-build`: Skips the entire build phase.
- `--no-viewer`: Disables the viewer.

Alternatively, you can use CMake directly:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Then, if you want to install the Python package, run from the root of the repository:

```bash
pip install -e .
```

## Running Tests

Tissu includes a comprehensive test suite for both C++ and Python. I recommend using the provided Python utility script
to run the tests.

```bash
python scripts/test.py 
```

### Tests Script Flags (`test.py`)

- `--no-test`: Skips the unit tests.
- `--no-integration`: Skips the integration tests.
- `--no-api-test`: Skips the API tests.
- `--bench`: Runs the benchmarks.

If you prefer to run the tests manually, you can use CTest for C++ tests and Pytest for Python tests.

```bash
ctest --test-dir build/tests --output-on-failure
```

````bash
pytest -v tests/python
````