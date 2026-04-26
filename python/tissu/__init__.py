import os
import sys
from pathlib import Path

current_file = Path(__file__).resolve()
package_dir = current_file.parent 

if str(package_dir) not in sys.path:
    sys.path.insert(0, str(package_dir))

project_root = current_file.parents[2]
build_dir = project_root / "build"
if build_dir.exists() and str(build_dir) not in sys.path:
    sys.path.append(str(build_dir))

try:
    import _cloth_sdk_core
    from _cloth_sdk_core import *
except ImportError as e:
    print(f"[Tissu] Error: C++ backend not found in {package_dir} or {build_dir}")
    raise e

from .engine import Simulation, Fabric, Material