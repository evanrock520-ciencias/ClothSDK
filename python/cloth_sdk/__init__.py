import os
import sys
from pathlib import Path

current_file = Path(__file__).resolve()
project_root = current_file.parents[2] 
build_dir = project_root / "build"

if build_dir.exists() and str(build_dir) not in sys.path:
    sys.path.append(str(build_dir))

try:
    import _cloth_sdk_core
    from _cloth_sdk_core import *
except ImportError as e:
    print(f"[ClothSDK] Error: C++ backend not found in {build_dir}")
    raise e

from .engine import Simulation, Fabric
