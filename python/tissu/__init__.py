import os
import sys
from pathlib import Path

project_root = Path(__file__).resolve().parents[2]
build_dir = project_root / "build"

if build_dir.exists() and str(build_dir) not in sys.path:
    sys.path.append(str(build_dir))

if os.name == "nt":
    # Add local build output directories for DLL resolution
    for config in ["Release", "Debug", ""]:
        for folder in ["bin", "lib"]:
            local_dll_dir = build_dir / folder / config if config else build_dir / folder
            if local_dll_dir.exists():
                os.add_dll_directory(str(local_dll_dir))

    vcpkg_root = os.environ.get("VCPKG_ROOT")

    if vcpkg_root:
        dll_dir = Path(vcpkg_root) / "installed" / "x64-windows" / "bin"

        if dll_dir.exists():
            os.add_dll_directory(str(dll_dir))

try:
    from . import _cloth_sdk_core  # noqa: F401
    from ._cloth_sdk_core import *  # noqa: F403

except ImportError as e:
    print(f"[Tissu] Import error: {e}")
    raise

from .engine import Fabric, Material, Simulation  # noqa: E402

__all__ = ["Fabric", "Material", "Simulation"]
