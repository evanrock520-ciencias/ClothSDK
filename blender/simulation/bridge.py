from __future__ import annotations

from pathlib import Path

import numpy as np

ADDON_PATH = Path(__file__).resolve().parents[1]
LIBS_PATH = str(ADDON_PATH / "libs")
if LIBS_PATH not in __import__("sys").path:
    __import__("sys").path.insert(0, LIBS_PATH)

from _cloth_sdk_core import (  # noqa: E402
    AlembicExporter,
    Cloth,
    ClothMaterial,
    GravityForce,
    Logger,
    Solver,
    StateSerializer,
    World,
)


class Simulation:
    def __init__(
        self,
        substeps: int = 10,
        iterations: int = 2,
        gravity: float = -9.81,
        thickness: float = 0.02,
    ):
        self.world = World()
        self.solver = Solver()
        self._substeps = substeps
        self._iterations = iterations
        self._thickness = thickness
        self._gravity = gravity
        self._wind = [0.0, 0.0, 0.0]
        self._air_thickness = 0.01
        self._collision_compliance = self.solver.get_collision_compliance()
        self._static_friction = self.solver.get_static_friction()
        self._dynamic_friction = self.solver.get_dynamic_friction()

        # Setup gravity force in World
        self._gravity_vector = np.array([0.0, float(gravity), 0.0], dtype=np.float64)
        self._gravity_force = GravityForce(self._gravity_vector)
        self.world.add_force(self._gravity_force)

        # Track simulation objects
        self.cloth_objects = {}
        self._aero_forces = {}
        self._colliders = {}

    @property
    def substeps(self):
        return self._substeps

    @substeps.setter
    def substeps(self, value):
        print(f"--> [Bridge] Change substeps to: {value}")
        self._substeps = value
        self.solver.set_substeps(value)

    @property
    def iterations(self):
        return self._iterations

    @iterations.setter
    def iterations(self, value):
        print(f"--> [Bridge] Change iterations to: {value}")
        self._iterations = value
        self.solver.set_iterations(value)

    @property
    def thickness(self):
        return self._thickness

    @thickness.setter
    def thickness(self, value):
        print(f"--> [Bridge] Change thickness to: {value}")
        self._thickness = value
        self.world.set_thickness(value)

    @property
    def gravity(self):
        return self._gravity

    @gravity.setter
    def gravity(self, value):
        print(f"--> [Bridge] Change gravity to: {value}")
        self._gravity = value
        self._gravity_vector = np.array([0.0, value, 0.0], dtype=np.float64)
        self.world.set_gravity(self._gravity_vector)
        if hasattr(self, "_gravity_force") and self._gravity_force is not None:
            self._gravity_force.set_gravity(self._gravity_vector)

    @property
    def wind(self):
        return self._wind

    @wind.setter
    def wind(self, value):
        val_list = [float(x) for x in value]
        print(f"--> [Bridge] Change wind to: {val_list}")
        self._wind = np.array(val_list, dtype=np.float64)
        # Convert wind from Blender Z-up to Tissu Y-up space:
        # Tissu X = Blender X
        # Tissu Y = Blender Z
        # Tissu Z = -Blender Y
        tissu_wind = np.array([val_list[0], val_list[2], -val_list[1]], dtype=np.float64)
        self.world.set_wind(tissu_wind)
        for force in self._aero_forces.values():
            force.set_wind(tissu_wind)

    @property
    def air_thickness(self):
        return self._air_thickness

    @air_thickness.setter
    def air_thickness(self, value):
        print(f"--> [Bridge] Change air thickness to: {value}")
        self._air_thickness = value
        self.world.set_air_density(value)
        for force in self._aero_forces.values():
            force.set_air_density(value)

    @property
    def collision_compliance(self):
        return self._collision_compliance

    @collision_compliance.setter
    def collision_compliance(self, value):
        print(f"--> [Bridge] Change collision compliance to: {value}")
        self._collision_compliance = value
        self.solver.set_collision_compliance(value)

    @property
    def static_friction(self):
        return self._static_friction

    @static_friction.setter
    def static_friction(self, value):
        print(f"--> [Bridge] Change static friction to: {value}")
        self._static_friction = value
        self.solver.set_static_friction(value)

    @property
    def dynamic_friction(self):
        return self._dynamic_friction

    @dynamic_friction.setter
    def dynamic_friction(self, value):
        print(f"--> [Bridge] Change dynamic friction to: {value}")
        self._dynamic_friction = value
        self.solver.set_dynamic_friction(value)

    def get_positions(self) -> np.ndarray:
        particles = self.solver.get_particles()
        return np.array([p.get_position() for p in particles], dtype=np.float64)

    def step(self, dt: float = 1.0 / 60.0):
        self.solver.update(self.world, dt)

    def save_state(self, path: str):
        StateSerializer.save(path, self.solver, self.world)

    def load_state(self, path: str):
        StateSerializer.load(path, self.solver, self.world)

    def bake_alembic(
        self,
        filepath: str,
        start_frame: int = 0,
        end_frame: int = 120,
        fps: float = 60.0,
    ) -> bool:
        if not self.cloth_objects:
            raise RuntimeError("No cloth objects found in simulation to bake.")

        exporter = AlembicExporter()
        dt = 1.0 / fps

        names = list(self.cloth_objects.keys())
        global_indices_list = []
        particle_indices_list = []
        for name in names:
            cloth_obj = self.cloth_objects[name]
            global_indices_list.append(cloth_obj.get_triangles())
            particle_ids = cloth_obj.instance.get_particle_indices()
            particle_indices_list.append(np.array(particle_ids, dtype=np.int32))

        print(f"--> [Bridge] Baking simulation to {filepath}...")

        if not exporter.open(
            filepath,
            names,
            self.get_positions(),
            global_indices_list,
            particle_indices_list,
        ):
            print(f"--> [Bridge] Failed to create Alembic file: {filepath}")
            return False

        total_frames = end_frame - start_frame

        for frame_idx in range(total_frames):
            self.step(dt)
            current_time = frame_idx * dt
            exporter.write_frame(self.get_positions(), current_time)

        exporter.close()
        print(f"--> [Bridge] Bake completed successfully: {filepath}")
        return True


class Fabric:
    def __init__(self, name: str, material: Material) -> None:
        self.name = name
        self.material = material
        native_mat = ClothMaterial(
            material.density,
            material.structural,
            material.shear,
            material.bending,
        )
        self.instance = Cloth(name, native_mat)
        self.solver = None
        self._pins = []

    def get_positions(self) -> np.ndarray:
        if self.solver is None:
            return np.empty((0, 3), dtype=np.float64)
        all_particles = self.solver.get_particles()
        my_indices = self.instance.get_particle_indices()
        return np.array(
            [all_particles[idx].get_position() for idx in my_indices],
            dtype=np.float64,
        )

    def get_triangles(self):
        return self.instance.get_triangles()


class Material:
    def __init__(self, density: float, structural: float, shear: float, bending: float) -> None:
        self._density = density
        self._structural = structural
        self._shear = shear
        self._bending = bending

    @property
    def density(self):
        return self._density

    @density.setter
    def density(self, value):
        self._density = value

    @property
    def structural(self):
        return self._structural

    @structural.setter
    def structural(self, value):
        self._structural = value

    @property
    def shear(self):
        return self._shear

    @shear.setter
    def shear(self, value):
        self._shear = value

    @property
    def bending(self):
        return self._bending

    @bending.setter
    def bending(self, value):
        self._bending = value

    @classmethod
    def from_dict(cls, data: dict):
        return cls(
            density=data.get("density", 0.1),
            structural=data.get("structural", 1e-9),
            shear=data.get("shear", 1e-8),
            bending=data.get("bending", 0.01),
        )


tissu_sim_instance = None


def get_simulation() -> Simulation:
    global tissu_sim_instance
    if tissu_sim_instance is None:
        tissu_sim_instance = Simulation()
    return tissu_sim_instance


classes = [Material, Fabric, Simulation]


def register():
    global tissu_sim_instance
    tissu_sim_instance = Simulation()
    Logger.info("Tissu simulation bridge registered.")


def unregister():
    global tissu_sim_instance
    if tissu_sim_instance is not None:
        tissu_sim_instance = None
    Logger.info("Tissu simulation bridge unregistered.")
