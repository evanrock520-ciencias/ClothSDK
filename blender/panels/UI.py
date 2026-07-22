import contextlib

import bpy


class VIEW3D_PT_Simulation(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Simulation"
    bl_label = "Simulation"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "PHYSICS"

    def draw(self, context):
        pass


class VIEW3D_PT_Solver(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Solver"
    bl_label = "Solver"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Simulation"
    bl_icon = "SETTINGS"

    def draw(self, context):
        layout = self.layout
        solverProps = context.scene.solver_props
        layout.prop(solverProps, "substeps")
        layout.prop(solverProps, "iterations")
        layout.prop(solverProps, "thickness")
        layout.prop(solverProps, "collision_compliance")
        layout.prop(solverProps, "static_friction")
        layout.prop(solverProps, "dynamic_friction")


class VIEW3D_PT_Environment(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Environment"
    bl_label = "Environment"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Simulation"
    bl_icon = "WORLD"

    def draw(self, context):
        layout = self.layout
        worldProps = context.scene.world_props
        layout.prop(worldProps, "gravity")
        layout.prop(worldProps, "wind")
        layout.prop(worldProps, "air_thickness")


class VIEW3D_PT_State(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_State"
    bl_label = "State"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Simulation"

    def draw(self, context):
        layout = self.layout
        from ..simulation import session

        row_a = layout.row(align=True)
        if session.is_simulating_live:
            row_a.operator("tissu.simulate", text="Stop Live", icon="CANCEL")
        else:
            row_a.operator("tissu.simulate", text="Simulate Live", icon="PLAY")
        row_a.operator("tissu.reset_simulation", text="Reset", icon="FILE_REFRESH")

        layout.operator("tissu.bake", text="Bake Alembic Cache", icon="EXPORT")

        row_b = layout.row(align=True)
        row_b.operator("tissu.save_state", text="Save State", icon="FILE_IMAGE")
        row_b.operator("tissu.load_state", text="Load State", icon="FILE_FOLDER")


class VIEW3D_PT_Material(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Material"
    bl_label = "Material"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "MATERIAL"

    def draw(self, context):
        layout = self.layout
        materialProps = context.scene.material_props
        layout.prop(materialProps, "preset")
        col = layout.column()
        col.enabled = materialProps.preset == "CUSTOM"
        col.prop(materialProps, "density")
        col.prop(materialProps, "structural")
        col.prop(materialProps, "shear")
        col.prop(materialProps, "bending")
        row = layout.row(align=True)
        row.operator("tissu.load_material", text="Load", icon="IMPORT")
        row.operator("tissu.save_material", text="Save", icon="EXPORT")


class VIEW3D_PT_Colliders(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Colliders"
    bl_label = "Colliders"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "MESH_ICOSPHERE"

    def draw(self, context):
        layout = self.layout
        obj = context.active_object

        layout.operator(
            "tissu.mark_as_collider",
            text="Mark as Collider",
            icon="MESH_ICOSPHERE",
        )
        layout.operator("tissu.remove_collider", text="Remove Collider", icon="X")

        if obj and obj.tissu_is_collider:
            box = layout.box()
            box.label(text=f"Collider: {obj.name}", icon="OBJECT_DATA")
            box.prop(obj, "tissu_collider_type")
            box.prop(obj, "tissu_collider_friction")


class VIEW3D_PT_Stitches(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Stitches"
    bl_label = "Stitches"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "UV"

    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.add_seam", text="Add Seam", icon="UV_EDGESEL")
        layout.operator("tissu.remove_seam", text="Remove Seam", icon="X")


class VIEW3D_PT_Pins(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Pins"
    bl_label = "Pins"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "PINNED"

    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.add_pin", text="Pin Selected", icon="PINNED")
        layout.operator("tissu.unpin", text="Unpin Selected", icon="UNPINNED")


class VIEW3D_PT_Patterns(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Patterns"
    bl_label = "Patterns"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "MOD_BUILD"

    def draw(self, context):
        layout = self.layout
        layout.label(text="A panel for patterns")


class VIEW3D_PT_Fabrics(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Fabrics"
    bl_label = "Fabrics"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "MESH_DATA"

    def draw(self, context):
        layout = self.layout
        obj = context.active_object

        layout.operator("tissu.mark_as_fabric", text="Mark as Fabric", icon="ADD")
        layout.operator("tissu.unmark_as_fabric", text="Unmark as Fabric", icon="REMOVE")

        if obj and obj.tissu_is_fabric:
            box = layout.box()
            box.prop(obj, "tissu_volume_preservation")


class VIEW3D_PT_NewPattern(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_NewPattern"
    bl_label = "Create Pattern"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Patterns"
    bl_icon = "ADD"

    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.new_pattern", text="New Pattern", icon="ADD")
        layout.operator(
            "tissu.from_selected_mesh",
            text="From Selected Mesh",
            icon="MESH_DATA",
        )


class VIEW3D_PT_Remesh(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Remesh"
    bl_label = "Remesh Pattern"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Patterns"
    bl_icon = "MOD_REMESH"

    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.remesh", text="Remesh", icon="MOD_REMESH")


class VIEW3D_PT_Attachments(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Attachments"
    bl_label = "Attachments"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Pins"

    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.attach_to_collider", text="Attach to Collider", icon="CONSTRAINT")


classes = [
    VIEW3D_PT_Simulation,
    VIEW3D_PT_Solver,
    VIEW3D_PT_Environment,
    VIEW3D_PT_State,
    VIEW3D_PT_Material,
    VIEW3D_PT_Colliders,
    VIEW3D_PT_Patterns,
    VIEW3D_PT_Fabrics,
    VIEW3D_PT_NewPattern,
    VIEW3D_PT_Remesh,
    VIEW3D_PT_Pins,
    VIEW3D_PT_Stitches,
    VIEW3D_PT_Attachments,
]


def register():
    for cls in classes:
        with contextlib.suppress(RuntimeError):
            bpy.utils.unregister_class(cls)
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        with contextlib.suppress(RuntimeError):
            bpy.utils.unregister_class(cls)
