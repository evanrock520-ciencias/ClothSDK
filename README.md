# 🧵 Tissu

![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)
![Language](https://img.shields.io/badge/Programming%20Language-Python-red)
![License](https://img.shields.io/badge/License-Apache%202.0-orange.svg)
![Physics](https://img.shields.io/badge/Physics-XPBD-green.svg)

**Tissu** is a C++ cloth simulation SDK designed to integrate seamlessly into Digital Content Creation (DCC) tools such as Blender. It was developed to make high-fidelity cloth simulation as accessible as possible for technical artists and developers.

By combining a high-performance **C++ core** with flexible **Python bindings**, Tissu allows you to script complex simulations, define materials via JSON, and export results directly to Alembic (.abc).

![Blender Simulation](docs/videos/blender_cloth.gif)

---

## 🛠️ Build and Installation

### 1. Clone Repositorie

Clone the repository is as simple as

```bash
git clone https://github.com/evanrock520-ciencias/Tissu.git
cd Tissu
```

### 2. Compile the SDK

Build the shared library and the standalone viewer using CMake.

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4 
```

### 3. Python Environment Setup

To import the library in your scripts, you must add the project path and the build artifact path to your `PYTHONPATH`.

**Linux / macOS:**

```bash
export PYTHONPATH=$PYTHONPATH:$(pwd)/python:$(pwd)/build
```

**Windows (PowerShell):**

```powershell
$env:PYTHONPATH = "$env:PYTHONPATH;$(Get-Location)\python;$(Get-Location)\build\Release"
```

> **Note:** Run this from the root of the repo

---

## 🚀 Getting Started with Python

Using the Python API, we can create a simulation scene, pin vertices, apply materials, and bake the result to a cache file.

Create a file named `simulation.py` on `examples` directorie:

```python
from tissu import Simulation

def run_falling_curtain():
# 1. Initialize Simulation Environment
	sim = Simulation(substeps=15, iterations=3, gravity=-9.81, thickness=0.05)

	# 2. Add Forces 
	sim.wind[5.0, 0.0, 0.0]

	# 3. Add colliders
	sim.add_floor(friction=0.5)

	# 4. Add fabric
	curtain = sim.create_grid(
		name="curtain",
		rows=80,
		cols=80,
		spacing=0.05,
		material="silk"
	)

	curtain.pin_top_corners()
	# 5. Export simulation
	sim.bake_alembic(
		filepath="sim.abc"
	)
    
if __name__ == "__main__":
    run_falling_curtain()
```

You can run the script above by writting.

```bash
python3 -m examples.simulation
```

---

## Viewer

![Viewer](docs/videos/viewer.gif)

>Note: Preview of a silk cloth.

---

## 🎨 Blender Integration

Since Tissu exports standard Alembic files, visualizing the result is straightforward:

1.  Run the Python script above to generate the `.abc` file.
2.  Open **Blender**.
3.  Go to **File > Import > Alembic (.abc)**.
4.  Select `data/animations/falling.abc`.
5.  Press **Play**.

![Example of Blender Animation](docs/videos/curtain.gif)

---

## 📄 License

This project is licensed under the Apache 2.0 License - see the [LICENSE](LICENSE) file for details.
