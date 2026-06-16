import bpy


class VIEW3D_PT_Simulation(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Simulation"
    bl_label = "Simulation"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"

    def draw(self, context):
        layout = self.layout


class VIEW3D_PT_Solver(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Solver"
    bl_label = "Solver"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Simulation"

    def draw(self, context):
        layout = self.layout
        solverProps = context.scene.solver_props
        layout.prop(solverProps, "substeps")
        layout.prop(solverProps, "iterations")
        layout.prop(solverProps, "thickness")


class VIEW3D_PT_World(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_World"
    bl_label = "World"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_parent_id = "VIEW3D_PT_Simulation"

    def draw(self, context):
        layout = self.layout
        worldProps = context.scene.world_props
        layout.prop(worldProps, "gravity")
        layout.prop(worldProps, "wind")
        layout.prop(worldProps, "air_thickness")


class VIEW3D_PT_Material(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Material"
    bl_label = "Material"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"

    def draw(self, context):
        layout = self.layout
        materialProps = context.scene.material_props
        layout.prop(materialProps, "density")
        layout.prop(materialProps, "structural")
        layout.prop(materialProps, "shear")
        layout.prop(materialProps, "bending")
        layout.prop(materialProps, "preset")
        row = layout.row(align=True)
        row.operator("tissu.load_material", text="Load", icon="IMPORT")
        row.operator("tissu.save_material", text="Save", icon="EXPORT")


class VIEW3D_PT_Colliders(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Colliders"
    bl_label = "Colliders"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    
    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.mark_as_collider", text="Mark as Collider")
        layout.operator("tissu.remove_collider", text="Remove Collider")


class VIEW3D_PT_Stitches(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Stitches"
    bl_label = "Stitches"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    
    def draw(self, context):
        layout = self.layout
        layout.label(text="A panel for stitches")
        layout.operator("tissu.add_seam", text="Add Seam")
        layout.operator("tissu.remove_seam", text="Remove Seam")


class VIEW3D_PT_Pins(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Pins"
    bl_label = "Pins"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    
    def draw(self, context):
        layout = self.layout
        layout.label(text="A panel for pins")
        layout.operator("tissu.add_pin", text="Pin Selected")
        layout.operator("tissu.unpin", text="Unpin Selected")


class VIEW3D_PT_Patterns(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Patterns"
    bl_label = "Patterns"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    
    def draw(self, context):
        layout = self.layout
        layout.label(text="A panel for patterns")


classes = [
    VIEW3D_PT_Simulation,
    VIEW3D_PT_Solver,
    VIEW3D_PT_World,
    VIEW3D_PT_Material,
    VIEW3D_PT_Colliders,
    VIEW3D_PT_Patterns, 
    VIEW3D_PT_Pins,
    VIEW3D_PT_Stitches
]


def register():
    for cls in classes:
        try:
            bpy.utils.unregister_class(cls)
        except RuntimeError:
            pass
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        try:
            bpy.utils.unregister_class(cls)
        except RuntimeError:
            pass