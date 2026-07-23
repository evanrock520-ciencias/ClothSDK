import contextlib
import json

import bmesh
import bpy


def get_seams(scene):
    try:
        return json.loads(scene.tissu_seams)
    except Exception:
        return []


def save_seams(scene, seams):
    scene.tissu_seams = json.dumps(seams)


class TISSU_OT_SaveMaterial(bpy.types.Operator):
    bl_idname = "tissu.save_material"
    bl_label = "Save Material"
    bl_options = {"REGISTER"}

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    filter_glob: bpy.props.StringProperty(default="*.json", options={"HIDDEN"})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        mat_props = context.scene.material_props
        data = {
            "density": mat_props.density,
            "structural_compliance": mat_props.structural,
            "shear_compliance": mat_props.shear,
            "bending_compliance": mat_props.bending,
        }
        try:
            with open(self.filepath, "w") as f:
                json.dump(data, f, indent=4)
            self.report({"INFO"}, f"Material saved to {self.filepath}")
            return {"FINISHED"}
        except Exception as e:
            self.report({"ERROR"}, f"Failed to save material: {e}")
            return {"CANCELLED"}


class TISSU_OT_LoadMaterial(bpy.types.Operator):
    bl_idname = "tissu.load_material"
    bl_label = "Load Material"
    bl_options = {"REGISTER"}

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    filter_glob: bpy.props.StringProperty(default="*.json", options={"HIDDEN"})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        try:
            with open(self.filepath) as f:
                data = json.load(f)
            mat_props = context.scene.material_props
            mat_props.preset = "CUSTOM"

            if "density" in data:
                mat_props.density = data["density"]
            if "structural_compliance" in data:
                mat_props.structural = data["structural_compliance"]
            elif "structural" in data:
                mat_props.structural = data["structural"]
            if "shear_compliance" in data:
                mat_props.shear = data["shear_compliance"]
            elif "shear" in data:
                mat_props.shear = data["shear"]
            if "bending_compliance" in data:
                mat_props.bending = data["bending_compliance"]
            elif "bending" in data:
                mat_props.bending = data["bending"]

            self.report({"INFO"}, f"Material loaded from {self.filepath}")
            return {"FINISHED"}
        except Exception as e:
            self.report({"ERROR"}, f"Failed to load material: {e}")
            return {"CANCELLED"}


class TISSU_OT_MarkAsCollider(bpy.types.Operator):
    bl_idname = "tissu.mark_as_collider"
    bl_label = "Mark as Collider"
    bl_options = {"REGISTER"}

    @classmethod
    def poll(cls, context):
        return any(obj.type == "MESH" for obj in context.selected_objects)

    def execute(self, context):
        for obj in context.selected_objects:
            if obj.type == "MESH":
                obj.tissu_is_collider = True
                obj.tissu_collider_type = "MESH"
                print(f"{obj.name} marked as collider.")
        return {"FINISHED"}


class TISSU_OT_RemoveCollider(bpy.types.Operator):
    bl_idname = "tissu.remove_collider"
    bl_label = "Remove Collider"
    bl_options = {"REGISTER"}

    @classmethod
    def poll(cls, context):
        if len(context.selected_objects) < 1:
            return False

        return all(obj.tissu_is_collider for obj in context.selected_objects)

    def execute(self, context):
        for obj in context.selected_objects:
            obj.tissu_is_collider = False
            print(f"{obj.name} is no longer a collider.")
        return {"FINISHED"}


class TISSU_OT_AddPin(bpy.types.Operator):
    bl_idname = "tissu.add_pin"
    bl_label = "Pin Selected"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        if context.mode != "EDIT_MESH":
            return False
        for obj in context.selected_objects:
            if obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                for v in bm.verts:
                    if v.select:
                        return True
        return False

    def execute(self, context):
        for obj in context.selected_objects:
            if obj and obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                vg = obj.vertex_groups.get("Tissu_Pins")
                if not vg:
                    vg = obj.vertex_groups.new(name="Tissu_Pins")

                deform_layer = bm.verts.layers.deform.active
                if not deform_layer:
                    deform_layer = bm.verts.layers.deform.new()

                group_idx = vg.index
                count = 0
                for v in bm.verts:
                    if v.select:
                        v[deform_layer][group_idx] = 1.0
                        count += 1
                bmesh.update_edit_mesh(obj.data)
                self.report({"INFO"}, f"Pinned {count} vertices in {obj.name}.")
        return {"FINISHED"}


class TISSU_OT_Unpin(bpy.types.Operator):
    bl_idname = "tissu.unpin"
    bl_label = "Unpin Selected"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        if context.mode != "EDIT_MESH":
            return False
        for obj in context.selected_objects:
            if obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                for v in bm.verts:
                    if v.select:
                        return True
        return False

    def execute(self, context):
        for obj in context.selected_objects:
            if obj and obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                vg = obj.vertex_groups.get("Tissu_Pins")
                if vg:
                    deform_layer = bm.verts.layers.deform.active
                    if deform_layer:
                        group_idx = vg.index
                        count = 0
                        for v in bm.verts:
                            if v.select and group_idx in v[deform_layer]:
                                del v[deform_layer][group_idx]
                                count += 1
                        bmesh.update_edit_mesh(obj.data)
                        self.report(
                            {"INFO"},
                            f"Unpinned {count} vertices in {obj.name}.",
                        )
        return {"FINISHED"}


class TISSU_OT_AddSeam(bpy.types.Operator):
    bl_idname = "tissu.add_seam"
    bl_label = "Add Seam"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        if context.mode != "EDIT_MESH":
            return False

        selected_verts = []
        for obj in context.selected_objects:
            if obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                for v in bm.verts:
                    if v.select:
                        selected_verts.append((obj.name, v.index))
        return len(selected_verts) == 2

    def execute(self, context):
        selected_verts = []
        for obj in context.selected_objects:
            if obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                for v in bm.verts:
                    if v.select:
                        selected_verts.append((obj.name, v.index))
        if len(selected_verts) != 2:
            self.report({"WARNING"}, "Please select exactly two vertices.")
            return {"CANCELLED"}

        seams = get_seams(context.scene)
        new_seam = [
            selected_verts[0][0],
            selected_verts[0][1],
            selected_verts[1][0],
            selected_verts[1][1],
        ]

        for seam in seams:
            if (
                seam[0] == new_seam[0] and seam[1] == new_seam[1] and seam[2] == new_seam[2] and seam[3] == new_seam[3]
            ) or (
                seam[0] == new_seam[2] and seam[1] == new_seam[3] and seam[2] == new_seam[0] and seam[3] == new_seam[1]
            ):
                self.report({"INFO"}, "Seam already exists.")
                return {"FINISHED"}

        seams.append(new_seam)
        save_seams(context.scene, seams)
        self.report(
            {"INFO"},
            f"Added seam between {new_seam[0]}:{new_seam[1]} and {new_seam[2]}:{new_seam[3]}",  # noqa: E501
        )
        return {"FINISHED"}


class TISSU_OT_RemoveSeam(bpy.types.Operator):
    bl_idname = "tissu.remove_seam"
    bl_label = "Remove Seam"
    bl_options = {"REGISTER", "UNDO"}

    @classmethod
    def poll(cls, context):
        if context.mode != "EDIT_MESH":
            return False

        selected_verts = []
        for obj in context.selected_objects:
            if obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                for v in bm.verts:
                    if v.select:
                        selected_verts.append((obj.name, v.index))
        return len(selected_verts) == 2

    def execute(self, context):
        selected_verts = []
        for obj in context.selected_objects:
            if obj.type == "MESH" and obj.mode == "EDIT":
                bm = bmesh.from_edit_mesh(obj.data)
                for v in bm.verts:
                    if v.select:
                        selected_verts.append((obj.name, v.index))
        if len(selected_verts) != 2:
            self.report({"WARNING"}, "Please select exactly two vertices.")
            return {"CANCELLED"}

        seams = get_seams(context.scene)
        new_seams = []
        removed = False
        v1, v2 = selected_verts[0], selected_verts[1]

        for seam in seams:
            match1 = seam[0] == v1[0] and seam[1] == v1[1] and seam[2] == v2[0] and seam[3] == v2[1]
            match2 = seam[0] == v2[0] and seam[1] == v2[1] and seam[2] == v1[0] and seam[3] == v1[1]
            if match1 or match2:
                removed = True
            else:
                new_seams.append(seam)

        if removed:
            save_seams(context.scene, new_seams)
            self.report({"INFO"}, "Removed seam.")
        else:
            self.report({"WARNING"}, "No matching seam found to remove.")
        return {"FINISHED"}


class TISSU_OT_MarkAsFabric(bpy.types.Operator):
    bl_idname = "tissu.mark_as_fabric"
    bl_label = "Mark as Fabric"
    bl_options = {"REGISTER"}

    @classmethod
    def poll(cls, context):
        return any(obj.type == "MESH" for obj in context.selected_objects)

    def execute(self, context):
        for obj in context.selected_objects:
            if obj.type == "MESH":
                obj.tissu_is_fabric = True

                has_triangulate = any(m.type == "TRIANGULATE" for m in obj.modifiers)
                if not has_triangulate:
                    mod = obj.modifiers.new(name="Triangulate", type="TRIANGULATE")
                    mod.quad_method = "FIXED"

                print(f"{obj.name} marked as fabric.")
        return {"FINISHED"}


class TISSU_OT_UnmarkAsFabric(bpy.types.Operator):
    bl_idname = "tissu.unmark_as_fabric"
    bl_label = "Unmark as Fabric"
    bl_options = {"REGISTER"}

    @classmethod
    def poll(cls, context):
        if not context.selected_objects:
            return False
        return all(obj.type == "MESH" and obj.tissu_is_fabric for obj in context.selected_objects)

    def execute(self, context):
        for obj in context.selected_objects:
            obj.tissu_is_fabric = False
            print(f"{obj.name} unmarked as fabric.")
        return {"FINISHED"}


class TISSU_OT_NewPattern(bpy.types.Operator):
    bl_idname = "tissu.new_pattern"
    bl_label = "New Pattern"
    bl_options = {"REGISTER"}

    def execute(self, context):
        self.report({"INFO"}, "New Pattern is not implemented in the physics MVP.")
        return {"FINISHED"}


class TISSU_OT_PatternFromSelectedMesh(bpy.types.Operator):
    bl_idname = "tissu.from_selected_mesh"
    bl_label = "From Selected Mesh"
    bl_options = {"REGISTER"}

    @classmethod
    def poll(cls, context):
        return any(obj.type == "MESH" for obj in context.selected_objects)

    def execute(self, context):
        self.report(
            {"INFO"},
            "Pattern from selected mesh is not implemented in the physics MVP.",
        )
        return {"FINISHED"}


class TISSU_OT_Remesh(bpy.types.Operator):
    bl_idname = "tissu.remesh"
    bl_label = "Remesh"
    bl_options = {"REGISTER"}

    def execute(self, context):
        self.report({"INFO"}, "Remesh is not implemented in the physics MVP.")
        return {"FINISHED"}


class TISSU_OT_Bake(bpy.types.Operator):
    bl_idname = "tissu.bake"
    bl_label = "Bake Alembic Cache"
    bl_options = {"REGISTER"}

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    filter_glob: bpy.props.StringProperty(default="*.abc", options={"HIDDEN"})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        from ..simulation.bridge import get_simulation
        from ..simulation.session import sync_simulation

        sim = get_simulation()
        sync_simulation(context)

        if not sim.cloth_objects:
            self.report({"WARNING"}, "No fabric objects to bake.")
            return {"CANCELLED"}

        scene = context.scene
        start_frame = scene.frame_start
        end_frame = scene.frame_end
        fps = scene.render.fps

        self.report({"INFO"}, f"Baking simulation to {self.filepath}...")
        success = sim.bake_alembic(
            filepath=self.filepath,
            start_frame=start_frame,
            end_frame=end_frame,
            fps=fps,
        )

        if success:
            self.report({"INFO"}, "Bake completed successfully.")
            return {"FINISHED"}
        else:
            self.report({"ERROR"}, "Bake failed.")
            return {"CANCELLED"}


class TISSU_OT_Simulate(bpy.types.Operator):
    bl_idname = "tissu.simulate"
    bl_label = "Simulate"
    bl_options = {"REGISTER"}

    _timer = None

    def modal(self, context, event):
        from ..simulation import session

        if not session.is_simulating_live or event.type in {"ESC"}:
            self.cancel(context)
            return {"CANCELLED"}

        if event.type == "TIMER":
            from ..simulation.session import step_simulation

            step_simulation(context, 1.0 / 60.0)

            # Tag 3D views for redraw
            for area in context.screen.areas:
                if area.type == "VIEW_3D":
                    area.tag_redraw()

        return {"PASS_THROUGH"}

    def execute(self, context):
        from ..simulation import session

        if session.is_simulating_live:
            session.is_simulating_live = False
            return {"FINISHED"}

        from ..simulation.session import sync_simulation

        sync_simulation(context)
        session.is_simulating_live = True

        wm = context.window_manager
        self._timer = wm.event_timer_add(1.0 / 60.0, window=context.window)
        wm.modal_handler_add(self)
        print("[Tissu] Live simulation started. Press ESC to stop.")
        return {"RUNNING_MODAL"}

    def cancel(self, context):
        wm = context.window_manager
        if self._timer:
            wm.event_timer_del(self._timer)
            self._timer = None
        from ..simulation import session

        session.is_simulating_live = False
        print("[Tissu] Live simulation stopped.")


class TISSU_OT_ResetSimulation(bpy.types.Operator):
    bl_idname = "tissu.reset_simulation"
    bl_label = "Reset Simulation"
    bl_options = {"REGISTER"}

    def execute(self, context):
        from ..simulation import session

        if session.is_simulating_live:
            session.is_simulating_live = False

        restored = session.restore_fabric_rest_positions(context)
        session.reset_session()
        self.report({"INFO"}, f"Simulation reset. Restored {restored} fabric(s).")
        return {"FINISHED"}


class TISSU_OT_SaveState(bpy.types.Operator):
    bl_idname = "tissu.save_state"
    bl_label = "Save State"
    bl_options = {"REGISTER"}

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    filter_glob: bpy.props.StringProperty(default="*.json", options={"HIDDEN"})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        from ..simulation.bridge import get_simulation

        sim = get_simulation()
        if not sim.cloth_objects:
            self.report({"WARNING"}, "No cloth objects found in simulation to save state.")
            return {"CANCELLED"}

        try:
            sim.save_state(self.filepath)
            self.report({"INFO"}, f"Simulation state saved to {self.filepath}")
            return {"FINISHED"}
        except Exception as e:
            self.report({"ERROR"}, f"Failed to save simulation state: {e}")
            return {"CANCELLED"}


class TISSU_OT_LoadState(bpy.types.Operator):
    bl_idname = "tissu.load_state"
    bl_label = "Load State"
    bl_options = {"REGISTER"}

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")
    filter_glob: bpy.props.StringProperty(default="*.tissu", options={"HIDDEN"})

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {"RUNNING_MODAL"}

    def execute(self, context):
        from ..simulation.bridge import get_simulation

        sim = get_simulation()
        try:
            sim.load_state(self.filepath)
            self.report({"INFO"}, f"Simulation state loaded from {self.filepath}")
            return {"FINISHED"}
        except Exception as e:
            self.report({"ERROR"}, f"Failed to load simulation state: {e}")
            return {"CANCELLED"}


class TISSU_OT_AttachToCollider(bpy.types.Operator):
    bl_idname = "tissu.attach_to_collider"
    bl_label = "Attach to Collider"
    bl_options = {"REGISTER", "UNDO"}

    def execute(self, context):
        print("Attaching to collider...")
        return {"FINISHED"}


classes = [
    TISSU_OT_SaveMaterial,
    TISSU_OT_LoadMaterial,
    TISSU_OT_SaveState,
    TISSU_OT_LoadState,
    TISSU_OT_MarkAsCollider,
    TISSU_OT_RemoveCollider,
    TISSU_OT_MarkAsFabric,
    TISSU_OT_UnmarkAsFabric,
    TISSU_OT_AddPin,
    TISSU_OT_AddSeam,
    TISSU_OT_Unpin,
    TISSU_OT_RemoveSeam,
    TISSU_OT_NewPattern,
    TISSU_OT_PatternFromSelectedMesh,
    TISSU_OT_Remesh,
    TISSU_OT_Bake,
    TISSU_OT_Simulate,
    TISSU_OT_ResetSimulation,
    TISSU_OT_AttachToCollider,
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
