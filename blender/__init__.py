import bpy
from .pattern import properties, operators
from .panels import UI

bl_info = {
    "name": "Tissu",
    "author": "Evan Miranda",
    "version": (1, 0, 1),
    "blender": (5, 1, 0),
    "location": "View3D > N-Panel > Cloth",
    "description": "Cloth Simulation Addon",
    "category": "Physics",
}

modules = [
    properties,
    operators,
    UI,
]

def register():
    for module in modules:
        module.register()

def unregister():
    for module in reversed(modules):
        module.unregister()