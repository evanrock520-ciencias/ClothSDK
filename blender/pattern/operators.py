import bpy
import bmesh


class TISSU_OT_SaveMaterial(bpy.types.Operator):
    bl_idname = "tissu.save_material"
    bl_label = "Save Material"
    bl_options = {"REGISTER"}
    
    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    filename: bpy.props.StringProperty(subtype="FILE_NAME")
    filter_glob: bpy.props.StringProperty(default="*.json", options={'HIDDEN'})
    
    # Just Open Panels
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}
    
    def execute(self, context):
        return {'FINISHED'}


class TISSU_OT_LoadMaterial(bpy.types.Operator):
    bl_idname = "tissu.load_material"
    bl_label = "Load Material"
    bl_options = {"REGISTER"}
    
    # Just Open Panels
    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}
    
    def execute(self, context):
        return {'FINISHED'}

class TISSU_OT_MarkAsCollider(bpy.types.Operator):
    bl_idname = "tissu.mark_as_collider"
    bl_label = "Mark as Collider"
    bl_options = {"REGISTER"}
    
    @classmethod
    def poll(cls, context):
        return any(obj.type == 'MESH' for obj in context.selected_objects)


    def execute(self, context):
        for obj in context.selected_objects:
            if obj.type == 'MESH':
                obj.tissu_is_collider = True
                print(str(obj) + " is a collider.")
        return {'FINISHED'}


class TISSU_OT_RemoveCollider(bpy.types.Operator):
    bl_idname = "tissu.remove_collider"
    bl_label = "Remove Collider"
    bl_options = {"REGISTER"}
    
    @classmethod
    def poll(cls, context):
        if len(context.selected_objects) < 1:
            return False
        
        for obj in context.selected_objects:
            if obj.type != "MESH" or not obj.tissu_is_collider:
                return False
            
        return True

    def execute(self, context):
        for obj in context.selected_objects:
            obj.tissu_is_collider = False
            print(str(obj) + " is not a collider")
        return {'FINISHED'}


class TISSU_OT_AddPin(bpy.types.Operator):
    bl_idname = "tissu.add_pin"
    bl_label = "Pin Selected"
    bl_options = {"REGISTER"}
    
    def get_selected_vertices(self, obj):
        bm = bmesh.from_edit_mesh(obj.data)
        return [v.index for v in bm.verts if v.select]
    
    def execute(self, context):
        for obj in context.selected_objects:
            if obj and obj.type == 'MESH':
                selected = self.get_selected_vertices(obj)
                print(f"Vertices pinned: {selected}")
            # TODO: Pin on Simulation
            return {'FINISHED'}


class TISSU_OT_Unpin(bpy.types.Operator):
    bl_idname = "tissu.unpin"
    bl_label = "Unpin Selected"
    bl_options = {"REGISTER"}
    
    def get_selected_vertices(self, obj):
        bm = bmesh.from_edit_mesh(obj.data)
        return [v.index for v in bm.verts if v.select]


    def execute(self, context):
        for obj in context.selected_objects:
            if obj and obj.type == 'MESH':
                selected = self.get_selected_vertices(obj)
                print(f"Vertices unpinned: {selected}")
        # TODO: Unpin on Simulation
        return {'FINISHED'}


class TISSU_OT_AddSeam(bpy.types.Operator):
    bl_idname = "tissu.add_seam"
    bl_label = "Add Seam"
    bl_options = {"REGISTER"}


class TISSU_OT_RemoveSeam(bpy.types.Operator):
    bl_idname = "tissu.remove_seam"
    bl_label = "Remove Seam"
    bl_options = {"REGISTER"}


class TISSU_OT_NewPattern(bpy.types.Operator):
    bl_idname = "tissu.new_pattern"
    bl_label = "New Pattern"
    bl_options = {"REGISTER"}


class TISSU_OT_PatternFromSelectedMesh(bpy.types.Operator):
    bl_idname = "tissu.from_selected_mesh"
    bl_label = "From Selected Mesh"
    bl_options = {"REGISTER"}

    @classmethod
    def poll(cls, context):
        return any(obj.type == 'MESH' for obj in context.selected_objects)

class TISSU_OT_Remesh(bpy.types.Operator):
    bl_idname = "tissu.remesh"
    bl_label = "Remesh"
    bl_options = {"REGISTER"}

classes = [
    TISSU_OT_SaveMaterial, 
    TISSU_OT_LoadMaterial, 
    TISSU_OT_MarkAsCollider, 
    TISSU_OT_RemoveCollider, 
    TISSU_OT_AddPin, 
    TISSU_OT_AddSeam, 
    TISSU_OT_Unpin, 
    TISSU_OT_RemoveSeam,
    TISSU_OT_NewPattern,
    TISSU_OT_PatternFromSelectedMesh,
    TISSU_OT_Remesh
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

