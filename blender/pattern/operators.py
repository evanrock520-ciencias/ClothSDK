import bpy, os


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


class TISSU_OT_RemoveCollider(bpy.types.Operator):
    bl_idname = "tissu.remove_collider"
    bl_label = "Remove Collider"
    bl_options = {"REGISTER"}


class TISSU_OT_AddPin(bpy.types.Operator):
    bl_idname = "tissu.add_pin"
    bl_label = "Pin Selected"
    bl_options = {"REGISTER"}


class TISSU_OT_Unpin(bpy.types.Operator):
    bl_idname = "tissu.unpin"
    bl_label = "Unpin Selected"
    bl_options = {"REGISTER"}


class TISSU_OT_AddSeam(bpy.types.Operator):
    bl_idname = "tissu.add_seam"
    bl_label = "Add Seam"
    bl_options = {"REGISTER"}


class TISSU_OT_RemoveSeam(bpy.types.Operator):
    bl_idname = "tissu.remove_seam"
    bl_label = "Remove Seam"
    bl_options = {"REGISTER"}


classes = [TISSU_OT_SaveMaterial, TISSU_OT_LoadMaterial, TISSU_OT_MarkAsCollider, TISSU_OT_RemoveCollider, TISSU_OT_AddPin, TISSU_OT_AddSeam, TISSU_OT_Unpin, TISSU_OT_RemoveSeam]

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

