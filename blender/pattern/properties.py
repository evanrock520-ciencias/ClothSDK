import bpy


MATERIAL_PRESETS = {
    'COTTON':  {'density': 0.2,   'structural': 1e-9,  'shear': 1e-8,  'bending': 0.01},
    'DENIM':   {'density': 0.45,  'structural': 1e-10, 'shear': 1e-9,  'bending': 0.0005},
    'LEATHER': {'density': 0.7,   'structural': 0.0,   'shear': 1e-10, 'bending': 1e-6},
    'SILK':    {'density': 0.1,   'structural': 1e-9,  'shear': 1e-8,  'bending': 0.1},
    'SPANDEX': {'density': 0.15,  'structural': 0.005, 'shear': 0.005, 'bending': 0.1},
}

class SolverProperties(bpy.types.PropertyGroup):
    substeps: bpy.props.IntProperty(
        name="Substeps",
        default=10,
        min=1,
        max=80
    )
    iterations: bpy.props.IntProperty(
        name="Iterations",
        default=3,
        min=1,
        max=40
    )
    thickness: bpy.props.FloatProperty(
        name="Thickness",
        default=0.05,
        min=0.00001,
        max=1
    )


class WorldProperties(bpy.types.PropertyGroup):
    gravity: bpy.props.FloatProperty(
        name="Gravity",
        default=-9.81,
        min=-100.0,
        max=0.0
    )
    wind: bpy.props.FloatVectorProperty(
        name="Wind",
        default=[0.0, 0.0, 0.0]
    )
    air_thickness: bpy.props.FloatProperty(
        name="Air Thickness",
        default=0.1,
        min=0.0,
        max=1.0
    )


class MaterialProperties(bpy.types.PropertyGroup):
    preset: bpy.props.EnumProperty(
        name="Preset",
        items=[
            ('CUSTOM',  'Custom',  ''),
            ('SILK',    'Silk',    ''),
            ('COTTON',  'Cotton',  ''),
            ('DENIM',   'Denim',   ''),
            ('LEATHER', 'Leather', ''),
            ('SPANDEX', 'Spandex', ''),
        ],
        default='CUSTOM',
    )
    density: bpy.props.FloatProperty(
        name="Density",
        default=0.1,
        min=0.0,
        max=100.0
    )
    bending: bpy.props.FloatProperty(
        name="Bending Compliance",
        default=0.1,
        min=0.0,
        max=1.0
    )
    shear: bpy.props.FloatProperty(
        name="Shear Compliance",
        default=1e-8,
        min=0.0,
        max=1.0
    )
    structural: bpy.props.FloatProperty(
        name="Structural Compliance",
        default=1e-9,
        min=0.0,
        max=1.0
    )
    



classes = [SolverProperties, WorldProperties, MaterialProperties]


def register():
    for cls in classes:
        try:
            bpy.utils.unregister_class(cls)
        except RuntimeError:
            pass
        bpy.utils.register_class(cls)
    bpy.types.Scene.solver_props = bpy.props.PointerProperty(type=SolverProperties)
    bpy.types.Scene.world_props = bpy.props.PointerProperty(type=WorldProperties)
    bpy.types.Scene.material_props = bpy.props.PointerProperty(type=MaterialProperties)
    bpy.types.Object.tissu_is_collider = bpy.props.BoolProperty(
        name="Is Collider",
        default=False
    )


def unregister():
    for cls in reversed(classes):
        try:
            bpy.utils.unregister_class(cls)
        except RuntimeError:
            pass
    if hasattr(bpy.types.Scene, "solver_props"):
        del bpy.types.Scene.solver_props
    if hasattr(bpy.types.Scene, "world_props"):
        del bpy.types.Scene.world_props
    if hasattr(bpy.types.Scene, "material_props"):
        del bpy.types.Scene.material_props
    if hasattr(bpy.types.Object, "tissu_is_collider"):
        del bpy.types.Object.tissu_is_collider
