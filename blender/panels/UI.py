import bpy


class VIEW3D_PT_Simulation(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Simulation"
    bl_label = "Simulation"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "PHYSICS"

    def draw(self, context):
        layout = self.layout


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
        col = layout.column()
        col.enabled = (materialProps.preset == 'CUSTOM')
        col.prop(materialProps, "density")
        col.prop(materialProps, "structural")
        col.prop(materialProps, "shear")
        col.prop(materialProps, "bending")
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
    bl_icon = "MESH_ICOSPHERE"
    
    def draw(self, context):
        layout = self.layout
        layout.operator("tissu.mark_as_collider", text="Mark as Collider", icon="MESH_ICOSPHERE")
        layout.operator("tissu.remove_collider", text="Remove Collider", icon="X")


class VIEW3D_PT_Stitches(bpy.types.Panel):
    bl_idname = "VIEW3D_PT_Stitches"
    bl_label = "Stitches"
    bl_space_type = "VIEW_3D"
    bl_region_type = "UI"
    bl_category = "Tissu"
    bl_icon = "UV"
    
    def draw(self, context):
        layout = self.layout
        layout.label(text="A panel for stitches")
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
        layout.label(text="A panel for pins")
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
        layout.operator("tissu.from_selected_mesh", text="From Selected Mesh", icon="MESH_DATA")


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

classes = [
    VIEW3D_PT_Simulation,
    VIEW3D_PT_Solver,
    VIEW3D_PT_Environment,
    VIEW3D_PT_Material,
    VIEW3D_PT_Colliders,
    VIEW3D_PT_Patterns, 
    VIEW3D_PT_NewPattern,
    VIEW3D_PT_Remesh,
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