import contextlib

import bpy

MATERIAL_PRESETS = {
    "COTTON": {
        "density": 0.2,
        "structural": 1e-9,
        "shear": 1e-8,
        "bending": 0.01,
    },
    "DENIM": {
        "density": 0.45,
        "structural": 1e-10,
        "shear": 1e-9,
        "bending": 0.0005,
    },
    "LEATHER": {
        "density": 0.7,
        "structural": 0.0,
        "shear": 1e-10,
        "bending": 1e-6,
    },
    "SILK": {"density": 0.1, "structural": 1e-9, "shear": 1e-8, "bending": 0.1},
    "SPANDEX": {
        "density": 0.15,
        "structural": 0.005,
        "shear": 0.005,
        "bending": 0.1,
    },
}

# UPDATES


def update_substeps(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.substeps = self.substeps


def update_iterations(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.iterations = self.iterations


def update_thickness(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.thickness = self.thickness


def update_gravity(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.gravity = self.gravity


def update_wind(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.wind = self.wind


def update_air_thickness(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.air_thickness = self.air_thickness


def update_collision_compliance(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.collision_compliance = self.collision_compliance


def update_static_friction(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.static_friction = self.static_friction


def update_dynamic_friction(self, context):
    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    sim.dynamic_friction = self.dynamic_friction


class SolverProperties(bpy.types.PropertyGroup):
    substeps: bpy.props.IntProperty(name="Substeps", default=10, min=1, max=80, update=update_substeps)
    iterations: bpy.props.IntProperty(name="Iterations", default=3, min=1, max=40, update=update_iterations)
    thickness: bpy.props.FloatProperty(
        name="Thickness",
        default=0.05,
        min=0.00001,
        max=1,
        update=update_thickness,
    )
    collision_compliance: bpy.props.FloatProperty(
        name="Collision Compliance",
        default=0.0,
        min=0.0,
        max=1.0,
        update=update_collision_compliance,
    )
    static_friction: bpy.props.FloatProperty(
        name="Static Friction",
        default=0.3,
        min=0.0,
        max=1.0,
        update=update_static_friction,
    )
    dynamic_friction: bpy.props.FloatProperty(
        name="Dynamic Friction",
        default=0.2,
        min=0.0,
        max=1.0,
        update=update_dynamic_friction,
    )


class WorldProperties(bpy.types.PropertyGroup):
    gravity: bpy.props.FloatProperty(
        name="Gravity",
        default=-9.81,
        min=-100.0,
        max=0.0,
        update=update_gravity,
    )
    wind: bpy.props.FloatVectorProperty(name="Wind", default=[0.0, 0.0, 0.0], update=update_wind)
    air_thickness: bpy.props.FloatProperty(
        name="Air Thickness", default=0.1, min=0.0, max=1.0, update=update_air_thickness
    )


def _update_preset(self, context):
    if self.preset == "CUSTOM":
        return
    values = MATERIAL_PRESETS.get(self.preset)
    if values:
        self.density = values["density"]
        self.structural = values["structural"]
        self.shear = values["shear"]
        self.bending = values["bending"]


def _update_material_prop(self, context):
    from _cloth_sdk_core import ClothMaterial

    from ..simulation.bridge import get_simulation

    sim = get_simulation()
    native_mat = ClothMaterial(self.density, self.structural, self.shear, self.bending)

    for fabric in sim.cloth_objects.values():
        fabric.material.density = self.density
        fabric.material.structural = self.structural
        fabric.material.shear = self.shear
        fabric.material.bending = self.bending
        fabric.instance.set_material(native_mat)


class MaterialProperties(bpy.types.PropertyGroup):
    preset: bpy.props.EnumProperty(
        name="Preset",
        items=[
            ("CUSTOM", "Custom", ""),
            ("SILK", "Silk", ""),
            ("COTTON", "Cotton", ""),
            ("DENIM", "Denim", ""),
            ("LEATHER", "Leather", ""),
            ("SPANDEX", "Spandex", ""),
        ],
        default="CUSTOM",
        update=_update_preset,
    )
    density: bpy.props.FloatProperty(name="Density", default=0.1, min=0.0, max=100.0, update=_update_material_prop)
    bending: bpy.props.FloatProperty(
        name="Bending Compliance", default=0.1, min=0.0, max=1.0, update=_update_material_prop
    )
    shear: bpy.props.FloatProperty(
        name="Shear Compliance", default=1e-8, min=0.0, max=1.0, update=_update_material_prop
    )
    structural: bpy.props.FloatProperty(
        name="Structural Compliance", default=1e-9, min=0.0, max=1.0, update=_update_material_prop
    )


classes = [SolverProperties, WorldProperties, MaterialProperties]


def register():
    for cls in classes:
        with contextlib.suppress(RuntimeError):
            bpy.utils.unregister_class(cls)
        bpy.utils.register_class(cls)
    bpy.types.Scene.solver_props = bpy.props.PointerProperty(type=SolverProperties)
    bpy.types.Scene.world_props = bpy.props.PointerProperty(type=WorldProperties)
    bpy.types.Scene.material_props = bpy.props.PointerProperty(type=MaterialProperties)
    bpy.types.Object.tissu_is_collider = bpy.props.BoolProperty(name="Is Collider", default=False)
    bpy.types.Object.tissu_collider_type = bpy.props.EnumProperty(
        name="Collider Type",
        items=[
            ("MESH", "Mesh", "Triangle mesh collider"),
            ("PLANE", "Plane", "Infinite plane collider"),
            ("SPHERE", "Sphere", "Sphere collider"),
            ("CAPSULE", "Capsule", "Capsule collider"),
        ],
        default="MESH",
    )
    bpy.types.Object.tissu_collider_friction = bpy.props.FloatProperty(name="Friction", default=0.5, min=0.0, max=1.0)
    bpy.types.Object.tissu_is_fabric = bpy.props.BoolProperty(name="Is Fabric", default=False)
    bpy.types.Scene.tissu_seams = bpy.props.StringProperty(name="Tissu Seams", default="[]")


def unregister():
    for cls in reversed(classes):
        with contextlib.suppress(RuntimeError):
            bpy.utils.unregister_class(cls)
    if hasattr(bpy.types.Scene, "solver_props"):
        del bpy.types.Scene.solver_props
    if hasattr(bpy.types.Scene, "world_props"):
        del bpy.types.Scene.world_props
    if hasattr(bpy.types.Scene, "material_props"):
        del bpy.types.Scene.material_props
    if hasattr(bpy.types.Object, "tissu_is_collider"):
        del bpy.types.Object.tissu_is_collider
    if hasattr(bpy.types.Object, "tissu_is_fabric"):
        del bpy.types.Object.tissu_is_fabric
    if hasattr(bpy.types.Object, "tissu_collider_type"):
        del bpy.types.Object.tissu_collider_type
    if hasattr(bpy.types.Object, "tissu_collider_friction"):
        del bpy.types.Object.tissu_collider_friction
    if hasattr(bpy.types.Scene, "tissu_seams"):
        del bpy.types.Scene.tissu_seams
