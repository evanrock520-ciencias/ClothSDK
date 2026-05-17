import os
import sys
from pathlib import Path

project_root = Path(__file__).resolve().parents[2]
build_dir = project_root / "build"

if build_dir.exists() and str(build_dir) not in sys.path:
    sys.path.append(str(build_dir))

if os.name == "nt":
    vcpkg_root = os.environ.get("VCPKG_ROOT")

    if vcpkg_root:
        dll_dir = (
            Path(vcpkg_root)
            / "installed"
            / "x64-windows"
            / "bin"
        )

        if dll_dir.exists():
            os.add_dll_directory(str(dll_dir))

try:
    from . import _cloth_sdk_core
    from ._cloth_sdk_core import *

except ImportError as e:
    print(f"[Tissu] Import error: {e}")
    raise

from .engine import Simulation, Fabric, Material