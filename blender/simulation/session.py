import _cloth_sdk_core as sdk
import bpy
import numpy as np
from _cloth_sdk_core import AerodynamicForce, ClothMesh, GravityForce

from .bridge import Fabric, Material, get_simulation

is_simulating_live = False
_last_frame = 1
_is_handler_registered = False
_fabric_rest_positions = {}
_point_cache = {}


def _read_local_vertex_positions(obj):
    mesh = obj.data
    vertex_count = len(mesh.vertices)
    coords = np.empty(vertex_count * 3, dtype=np.float64)
    mesh.vertices.foreach_get("co", coords)
    return coords.reshape(-1, 3)


def _write_local_vertex_positions(obj, local_positions):
    mesh = obj.data
    if len(mesh.vertices) != len(local_positions):
        print(
            f"[Tissu] Warning: cannot restore {obj.name}, vertex count mismatch "  # noqa: E501
            f"({len(mesh.vertices)} vs {len(local_positions)})."
        )
        return False

    mesh.vertices.foreach_set("co", np.asarray(local_positions, dtype=np.float64).ravel())
    mesh.update()
    return True


def capture_fabric_rest_positions(context, force=False):
    fabric_objs = [obj for obj in context.scene.objects if obj.type == "MESH" and obj.tissu_is_fabric]
    fabric_names = {obj.name for obj in fabric_objs}

    for obj_name in list(_fabric_rest_positions.keys()):
        if obj_name not in fabric_names:
            del _fabric_rest_positions[obj_name]

    for obj in fabric_objs:
        if force or obj.name not in _fabric_rest_positions:
            _fabric_rest_positions[obj.name] = _read_local_vertex_positions(obj).copy()


def restore_fabric_rest_positions(context):
    restored_count = 0
    for obj_name, local_positions in _fabric_rest_positions.items():
        obj = context.scene.objects.get(obj_name)
        if obj and obj.type == "MESH" and _write_local_vertex_positions(obj, local_positions):
            restored_count += 1

    if restored_count:
        print(
            f"[Tissu] Restored rest positions for {restored_count} fabric object(s)."  # noqa: E501
        )

    return restored_count


def reset_session():
    anul_cache(0)
    sim = get_simulation()

    # Clear native objects
    sim.world.clear()
    sim.solver.clear()

    # Clear tracking dicts
    sim.cloth_objects.clear()
    sim._aero_forces.clear()
    sim._colliders.clear()

    # Re-initialize gravity since world.clear() removes all forces
    sim._gravity_vector = np.array([0.0, float(sim.gravity), 0.0], dtype=np.float64)
    sim._gravity_force = GravityForce(sim._gravity_vector)
    sim.world.add_force(sim._gravity_force)

    # Reset wind and air density to solver
    sim.world.set_wind(sim.wind)
    sim.world.set_air_density(sim.air_thickness)

    # Reset solver frame/time if needed
    sim.solver.soft_reset()

    print("[Tissu] Simulation session reset.")


def get_mesh_data(obj, local=False):
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(depsgraph)
    temp_mesh = eval_obj.to_mesh()

    # Find if "Tissu_Pins" group exists
    vg = obj.vertex_groups.get("Tissu_Pins")
    vg_idx = vg.index if vg else -1

    # Extract vertex coordinates
    vertex_count = len(temp_mesh.vertices)
    coords = np.empty(vertex_count * 3, dtype=np.float64)
    temp_mesh.vertices.foreach_get("co", coords)
    positions_raw = coords.reshape(-1, 3)

    if local:
        positions_space = positions_raw
    else:
        # Convert to world coordinates
        world_matrix = np.array(obj.matrix_world, dtype=np.float64)
        ones = np.ones((vertex_count, 1))
        positions_h = np.hstack((positions_raw, ones))
        positions_world = (positions_h @ world_matrix.T)[:, :3]
        positions_space = positions_world

    # Find pinned vertices in the evaluated mesh
    pinned_indices = []
    if vg_idx != -1:
        for v in temp_mesh.vertices:
            for g in v.groups:
                if g.group == vg_idx and g.weight > 0.0:
                    pinned_indices.append(v.index)
                    break

    # Extract triangle face indices
    face_count = len(temp_mesh.polygons)
    indices = np.empty(face_count * 3, dtype=np.int32)
    try:
        temp_mesh.polygons.foreach_get("vertices", indices)
    except RuntimeError:
        print(f"[Tissu] Error: {obj.name} is not fully triangulated. Please ensure the Triangulate modifier is active.")
        raise

    # Clean up evaluation mesh
    eval_obj.to_mesh_clear()

    # Convert from Blender Z-up to Tissu Y-up space:
    # Tissu X = Blender X
    # Tissu Y = Blender Z
    # Tissu Z = -Blender Y
    tissu_positions = np.zeros_like(positions_space)
    tissu_positions[:, 0] = positions_space[:, 0]
    tissu_positions[:, 1] = positions_space[:, 2]
    tissu_positions[:, 2] = -positions_space[:, 1]

    return tissu_positions.tolist(), indices.tolist(), pinned_indices


def update_blender_mesh(obj, world_positions):
    # Convert from Tissu Y-up to Blender Z-up space:
    # Blender X = Tissu X
    # Blender Y = -Tissu Z
    # Blender Z = Tissu Y
    blender_positions = np.zeros_like(world_positions)
    blender_positions[:, 0] = world_positions[:, 0]
    blender_positions[:, 1] = -world_positions[:, 2]
    blender_positions[:, 2] = world_positions[:, 1]

    # Convert from world to local coordinates
    local_matrix = np.array(obj.matrix_world.inverted(), dtype=np.float64)
    vertex_count = blender_positions.shape[0]
    ones = np.ones((vertex_count, 1))
    positions_h = np.hstack((blender_positions, ones))
    positions_local = (positions_h @ local_matrix.T)[:, :3]

    # Write back to mesh
    mesh = obj.data
    if len(mesh.vertices) == vertex_count:
        mesh.vertices.foreach_set("co", positions_local.ravel())
        mesh.update()
    else:
        print(
            f"[Tissu] Warning: mesh {obj.name} vertex count mismatch ({len(mesh.vertices)} vs {vertex_count}). Make sure the mesh is triangulated."  # noqa: E501
        )


def anul_cache(frame):
    for f in list(_point_cache.keys()):
        if f >= frame:
            del _point_cache[f]


def sync_simulation(context):
    sim = get_simulation()

    capture_fabric_rest_positions(context)
    reset_session()

    solver_props = context.scene.solver_props
    sim.substeps = solver_props.substeps
    sim.iterations = solver_props.iterations
    sim.thickness = solver_props.thickness
    sim.collision_compliance = solver_props.collision_compliance
    sim.static_friction = solver_props.static_friction
    sim.dynamic_friction = solver_props.dynamic_friction

    world_props = context.scene.world_props
    sim.gravity = world_props.gravity
    sim.wind = world_props.wind
    sim.air_thickness = world_props.air_thickness

    fabric_objs = [obj for obj in context.scene.objects if obj.type == "MESH" and obj.tissu_is_fabric]
    for obj in fabric_objs:
        print(f"[Tissu] Syncing fabric mesh: {obj.name}")
        positions, indices, pinned_indices = get_mesh_data(obj, local=False)

        mat_props = context.scene.material_props
        material = Material(
            density=mat_props.density,
            structural=mat_props.structural,
            shear=mat_props.shear,
            bending=mat_props.bending,
        )

        fabric = Fabric(obj.name, material)
        factory = ClothMesh()
        factory.build_from_mesh(positions, indices, fabric.instance, sim.solver, "")

        sim.world.add_cloth(fabric.instance)
        sim.cloth_objects[obj.name] = fabric
        fabric.solver = sim.solver

        if getattr(obj, "tissu_volume_preservation", False):
            fabric.enable_volume_preservation()

        faces = fabric.instance.get_aerofaces()
        aero = AerodynamicForce(faces, sim.wind, sim.air_thickness)
        sim.world.add_force(aero)
        sim._aero_forces[obj.name] = aero

        my_ids = fabric.instance.get_particle_indices()
        for v_idx in pinned_indices:
            global_id = my_ids[v_idx]
            t_pos = np.array(positions[v_idx], dtype=np.float64)
            sim.solver.add_pin(global_id, t_pos, 0.0)  # rigid pin

        fabric._pins = np.array(pinned_indices, dtype=np.int32)

    collider_objs = [obj for obj in context.scene.objects if obj.tissu_is_collider]

    for obj in collider_objs:
        print(f"[Tissu] Syncing collider: {obj.name} ({obj.tissu_collider_type})")
        friction = obj.tissu_collider_friction

        if obj.tissu_collider_type == "PLANE":
            local_origin = np.array([0.0, 0.0, 0.0], dtype=np.float64)
            local_normal = np.array([0.0, 1.0, 0.0], dtype=np.float64)
            sim.world.add_plane_collider(local_origin, local_normal, friction, obj.name)

        elif obj.tissu_collider_type == "SPHERE":
            # Local center is [0.0, 0.0, 0.0]
            local_center = np.array([0.0, 0.0, 0.0], dtype=np.float64)
            radius = (obj.dimensions.x + obj.dimensions.y + obj.dimensions.z) / 6.0
            sim.world.add_sphere_collider(local_center, radius, friction, obj.name)

        elif obj.tissu_collider_type == "CAPSULE":
            # Capsule segment length along local Z axis in Blender (local Y in Tissu)  # noqa: E501
            radius = (obj.dimensions.x + obj.dimensions.y) / 4.0
            height = obj.dimensions.z
            L = max(0.0, height - 2.0 * radius)

            # Local segment endpoints in Tissu space (Y-up)
            tissu_start = np.array([0.0, -L / 2.0, 0.0], dtype=np.float64)
            tissu_end = np.array([0.0, L / 2.0, 0.0], dtype=np.float64)

            sim.world.add_capsule_collider(tissu_start, tissu_end, radius, friction, obj.name)

        elif obj.tissu_collider_type == "MESH":
            # Local vertices
            positions, indices, _ = get_mesh_data(obj, local=True)
            triangles = [indices[i : i + 3] for i in range(0, len(indices), 3)]

            collider = sdk.MeshCollider(positions, triangles, friction)
            collider.set_name(obj.name)
            sim.world.add_collider(collider)

        # Store collider mapping
        sim._colliders[obj.name] = len(sim.world.get_colliders()) - 1

        # Apply the initial transform immediately
        translation = obj.matrix_world.to_translation()
        tissu_pos = np.array([translation[0], translation[2], -translation[1]], dtype=np.float64)

        import mathutils

        R_B2T = mathutils.Matrix(((1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, -1.0, 0.0)))
        R_T2B = R_B2T.inverted()
        R_Blender = obj.matrix_world.to_3x3()
        R_Tissu = R_B2T @ R_Blender @ R_T2B
        q = R_Tissu.to_quaternion()
        tissu_rot = np.array([q.x, q.y, q.z, q.w], dtype=np.float64)

        sim.world.move_collider(sim._colliders[obj.name], tissu_pos, tissu_rot)

    # 4. Sync Seams (Stitches)
    from ..pattern.operators import get_seams

    seams = get_seams(context.scene)
    for seam in seams:
        obj_name_A, v_idx_A, obj_name_B, v_idx_B = seam
        fabric_A = sim.cloth_objects.get(obj_name_A)
        fabric_B = sim.cloth_objects.get(obj_name_B)

        if fabric_A and fabric_B:
            mapA = fabric_A.instance.get_particle_indices()
            mapB = fabric_B.instance.get_particle_indices()
            if v_idx_A < len(mapA) and v_idx_B < len(mapB):
                gA = mapA[v_idx_A]
                gB = mapB[v_idx_B]
                sim.solver.add_stitch(int(gA), int(gB), 0.0)
                print(
                    f"[Tissu] Stitched {obj_name_A}:{v_idx_A} to {obj_name_B}:{v_idx_B}"  # noqa: E501
                )

    print(
        f"[Tissu] Simulation synced with {len(fabric_objs)} fabric(s) and {len(collider_objs)} collider(s)."  # noqa: E501
    )


def step_simulation(context, dt=1.0 / 60.0):
    sim = get_simulation()
    if not sim.cloth_objects:
        sync_simulation(context)
        if not sim.cloth_objects:
            return

    for obj_name, collider_idx in list(sim._colliders.items()):
        obj = context.scene.objects.get(obj_name)
        if obj:
            translation = obj.matrix_world.to_translation()
            tissu_pos = np.array(
                [translation[0], translation[2], -translation[1]],
                dtype=np.float64,
            )

            import mathutils

            R_B2T = mathutils.Matrix(((1.0, 0.0, 0.0), (0.0, 0.0, 1.0), (0.0, -1.0, 0.0)))
            R_T2B = R_B2T.inverted()
            R_Blender = obj.matrix_world.to_3x3()
            R_Tissu = R_B2T @ R_Blender @ R_T2B
            q = R_Tissu.to_quaternion()
            tissu_rot = np.array([q.x, q.y, q.z, q.w], dtype=np.float64)

            sim.world.move_collider(collider_idx, tissu_pos, tissu_rot)

    sim.solver.update(sim.world, dt)
    frame = context.scene.frame_current
    frame_data = {}
    for name, fabric in sim.cloth_objects.items():
        obj = context.scene.objects.get(name)
        if obj and obj.type == "MESH":
            positions = fabric.get_positions()
            if positions.size > 0:
                update_blender_mesh(obj, positions)
                frame_data[name] = {"pos": positions.copy()}

    if frame_data:
        _point_cache[frame] = frame_data


def frame_change_pre_handler(scene):
    global _last_frame, is_simulating_live
    if is_simulating_live:
        return

    context = bpy.context
    current_frame = scene.frame_current
    start_frame = scene.frame_start

    if current_frame == start_frame and current_frame not in _point_cache:
        sync_simulation(context)
        _last_frame = current_frame

        frame_data = {}
        sim = get_simulation()
        for name, fabric in sim.cloth_objects.items():
            obj = context.scene.objects.get(name)
            if obj and obj.type == "MESH":
                positions = fabric.get_positions()
                if positions.size > 0:
                    frame_data[name] = {"pos": positions.copy()}
        if frame_data:
            _point_cache[current_frame] = frame_data

    elif current_frame in _point_cache:
        # Playback from cache
        for name, data in _point_cache[current_frame].items():
            obj = context.scene.objects.get(name)
            if obj and obj.type == "MESH":
                update_blender_mesh(obj, data["pos"])
        _last_frame = current_frame

    elif current_frame == _last_frame + 1:
        fps = scene.render.fps
        dt = 1.0 / fps if fps > 0 else 1.0 / 24.0
        step_simulation(context, dt)
        _last_frame = current_frame

    else:
        _last_frame = current_frame
        print(f"[Tissu] Cannot simulate frame {current_frame} without previous frames. Go to start frame.")


def register():
    global _is_handler_registered
    if not _is_handler_registered:
        bpy.app.handlers.frame_change_pre.append(frame_change_pre_handler)
        _is_handler_registered = True
        print("[Tissu] Frame change handler registered.")


def unregister():
    global _is_handler_registered
    if _is_handler_registered:
        if frame_change_pre_handler in bpy.app.handlers.frame_change_pre:
            bpy.app.handlers.frame_change_pre.remove(frame_change_pre_handler)
        _is_handler_registered = False
        print("[Tissu] Frame change handler unregistered.")
    _fabric_rest_positions.clear()
    reset_session()
