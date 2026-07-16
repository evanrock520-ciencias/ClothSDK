import sys
from pathlib import Path

from .panels import UI
from .pattern import operators, properties
from .simulation import bridge, session

bl_info = {
    "name": "Tissu",
    "author": "Evan Miranda",
    "version": (1, 0, 1),
    "blender": (5, 1, 0),
    "location": "View3D > N-Panel > Cloth",
    "description": "Cloth Simulation Addon",
    "category": "Physics",
}

ADDON_PATH = Path(__file__).parent
LIBS_PATH = str(ADDON_PATH / "libs")
if LIBS_PATH not in sys.path:
    sys.path.insert(0, LIBS_PATH)

try:
    import _cloth_sdk_core as sdk
except ImportError as e:
    print(f"[Tissu] Failed to load native core: {e}")
    sdk = None

modules = [
    properties,
    operators,
    bridge,
    session,
    UI,
]


def register():
    for module in modules:
        module.register()


def unregister():
    for module in reversed(modules):
        module.unregister()
