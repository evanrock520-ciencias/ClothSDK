from __future__ import annotations
from typing import Iterator
from collections import defaultdict
from . import _cloth_sdk_core as sdk
import numpy as np
import os
from tqdm.auto import tqdm
import matplotlib.pyplot as plt
import imageio

class Simulation:
    def __init__(self, substeps: int = 10, iterations: int = 2, gravity: float = -9.81, thickness: float = 0.02):
        self.world = sdk.World()
        self.solver = sdk.Solver()
        
        self._gravity_vector = np.array([0.0, float(gravity), 0.0], dtype=np.float64)
        self._gravity_force = sdk.GravityForce(self._gravity_vector)
        self.world.add_force(self._gravity_force)
        
        self.cloth_objects = {}  
        self._aero_forces = {}   
        self._colliders = {}
        
        self.substeps = substeps
        self.iterations = iterations
        self.gravity = gravity
        self.thickness = thickness
        self.wind = [0.0, 0.0, 0.0]
        self.air_density = 0.1 
        self._collision_compliance = 0.0
        
        self.actions = defaultdict(list)
        
        # Energy
        self._ke_history = np.array([])
        self._pe_history = np.array([])
        self._recording = False
        self._last_dt = 1/60
        self._frame_counter = 0
        
        self.app = None

    @property
    def substeps(self):
        return self.solver.get_substeps()
    
    @substeps.setter
    def substeps(self, value: int):
        val = max(1, int(value))
        self.solver.set_substeps(val)

    @property
    def iterations(self):
        return self.solver.get_iterations()

    @iterations.setter
    def iterations(self, value: int):
        val = max(1, int(value))
        self.solver.set_iterations(val)

    @property
    def gravity(self):
        return self.world.get_gravity()[1]
    
    @gravity.setter
    def gravity(self, value: float):
        self._gravity_vector = np.array([0.0, float(value), 0.0], dtype=np.float64)
        self.world.set_gravity(self._gravity_vector)
        if self._gravity_force:
            self._gravity_force.set_gravity(self._gravity_vector)
        
    @property
    def wind(self):
        return self.world.get_wind()
    
    @wind.setter
    def wind(self, value):
        if len(value) != 3:
            raise ValueError("Wind must be a 3-element list or array [x, y, z]")
        wind_vector = np.array(value, dtype=np.float64)
        self.world.set_wind(wind_vector)
        for force in self._aero_forces.values():
            force.set_wind(wind_vector)

    @property
    def air_density(self):
        return self.world.get_air_density()

    @air_density.setter
    def air_density(self, value: float):
        dens = max(0.0, float(value))
        self.world.set_air_density(dens)
        for force in self._aero_forces.values():
            force.set_air_density(dens)
    
    @property
    def thickness(self):
        return self.world.get_thickness()
    
    @thickness.setter
    def thickness(self, value: float):
        val = max(0.001, float(value))
        self.world.set_thickness(val)
        
    @property
    def collision_compliance(self):
        return self._collision_compliance
    
    @property
    def fabrics(self):
        return self.cloth_objects.values()
    
    @property
    def time(self):
        return self.solver.get_time()
    
    @property
    def frame(self):
        return self.solver.get_frame()
    
    def kinetic_energy(self, dt=None):
        dt = dt or self._last_dt
        ke = np.empty(0)
        
        for particle in self.solver.get_particles():
            v = particle.get_velocity(dt)
            m = 1.0 / particle.get_inverse_mass() if particle.get_inverse_mass() > 0 else float('inf')
            k = 0.5 * m * np.dot(v, v) if m != float('inf') else 0.0
            
            ke = np.append(ke, k)
            
        return np.sum(ke)
    
    def potential_energy(self, dt=None):
        pe = np.empty(0)
        
        for particle in self.solver.get_particles():
            y = particle.get_position()[1]
            m = 1.0 / particle.get_inverse_mass() if particle.get_inverse_mass() > 0 else float('inf')
            p = m * abs(self.gravity) * y 
            
            pe = np.append(pe, p)
        
        return np.sum(pe)
        
    def start_recording(self):
        self._recording = True
        
    def stop_recording(self):
        self._recording = False
        
    def clear_history(self):
        self._ke_history = np.array([])
        self._pe_history = np.array([])
    
    def get_fabric(self, name: str) -> Fabric:
        if name not in self.cloth_objects:
            raise KeyError(f"Fabric '{name}' not found.")
        return self.cloth_objects[name]
    
    def stitch(self, fabricA: "Fabric", fabricB: "Fabric", 
                local_ids_A: np.ndarray, local_ids_B: np.ndarray, 
                compliance: float = 0.0) -> None:
        
        # TODO: Include variable lengths 
        if len(local_ids_A) != len(local_ids_B):
            raise ValueError("Local Id's Lenght Mistmatch.")
        
        mapA = np.array(fabricA.instance.get_particle_indices())
        mapB = np.array(fabricB.instance.get_particle_indices())
        global_ids_A = mapA[local_ids_A]
        global_ids_B = mapB[local_ids_B]
        
        for gA, gB in zip(global_ids_A, global_ids_B):
            self.solver.add_stitch(int(gA), int(gB), float(compliance))
        
    @classmethod
    def load_scene(cls, filepath: str) -> Simulation:
        sim = cls.__new__(cls)
        sim.world = sdk.World()
        sim.solver = sdk.Solver()
        sim.cloth_objects = {}
        sim._aero_forces = {}
        sim.app = None
        sim.actions = defaultdict(list)
        sim._ke_history = np.array([])
        sim._pe_history = np.array([])
        sim._recording = False
        sim._last_dt = 1/60
        sim._frame_counter = 0

        sdk.SceneLoader.load_scene(filepath, sim.solver, sim.world)
        sim._gravity_force = sdk.GravityForce(sim.world.get_gravity())
        sim.world.add_force(sim._gravity_force)

        for cloth in sim.world.get_cloths():
            fabric = Fabric.__new__(Fabric)
            fabric.name = cloth.get_name()
            fabric.instance = cloth
            fabric._solver = sim.solver
            fabric.material = Material()
            
            threshold = fabric.instance.get_pin().get_threshold() 
            compliance = fabric.instance.get_pin().get_compliance()
            
            match fabric.instance.get_pin().get_pin_mode():
                case sdk.PinMode.TOP_CORNERS:
                    fabric.pin_top_corners(threshold, compliance)
                case sdk.PinMode.BY_HEIGHT:
                    fabric.pin_by_height(threshold, compliance)
                case sdk.PinMode.NONE:
                    pass
            
            aero = sdk.AerodynamicForce(
                cloth.get_aerofaces(),
                sim.world.get_wind(),
                sim.world.get_air_density()
            )
            sim._aero_forces[fabric.name] = aero
            sim.world.add_force(aero)
            sim.cloth_objects[fabric.name] = fabric

        sdk.Logger.info(f"Scene loaded: {filepath}")
        return sim
    
    def on_frame(self, frame: int):
        def decorator(func, sim=self):
            self.actions[frame].append(func)
            sdk.Logger.info("Action added on " + str(frame) + " frame")
            return func
        return decorator
    
    def on_every(self, n: int, start: int = 0, end: int = None):
        def decorator(func, sim=self):
            stop = end if end is not None else 100_000
            for frame in range(start, stop, n):
                self.actions[frame].append(func)
            return func
        return decorator
            
    def on_range(self, start: int, end: int):
        def decorator(func, sim=self):
            for frame in range(start, end):
                self.actions[frame].append(func)
            return func
        return decorator
    
    def simulate(self, frames: int, dt: float = 1/60):
        for _ in tqdm(range(frames), desc="Simulating", unit="frame"):
            self.step(dt)
    
    def add_fabric(self, fabric: Fabric) -> None:
        """
        Adds a fabric to the simulation world.
        If a fabric with the same name already exists, it will be overwritten.

        Args:
            fabric: The Fabric instance to register.
        """
        if fabric.name in self.cloth_objects:
            sdk.Logger.warn(f"Fabric '{fabric.name}' already exists. Overwriting.")

        self.world.add_cloth(fabric.instance)

        aero = sdk.AerodynamicForce(
            fabric.instance.get_aerofaces(), 
            self.wind, 
            self.air_density
        )
        
        fabric._solver = self.solver
        self._aero_forces[fabric.name] = aero
        self.world.add_force(aero)

        self.cloth_objects[fabric.name] = fabric
        sdk.Logger.info(f"Successfully added fabric: {fabric.name}")
        
    def create_grid(self, name: str, rows: int, cols: int, spacing: float, material=None, translation=(0.0, 0.0, 0.0), rotation=(0.0, 0.0, 0.0, 1.0)) -> "Fabric":        
        if name in self.cloth_objects:
            raise ValueError(f"Fabric '{name}' already exists in simulation.")
        
        resolved = self._resolve_material(material)
        fabric = Fabric.grid(name, rows, cols, spacing, resolved, self.solver, translation, rotation)
        self.add_fabric(fabric)
        
        return fabric
    
    def create_from_obj(self, name: str, path: str, material=None, translation=(0.0, 0.0, 0.0), rotation=(0.0, 0.0, 0.0, 1.0)):
        if name in self.cloth_objects:
            raise ValueError(f"Fabric '{name}' already exists in simulation.")
        
        resolved = self._resolve_material(material)
        fabric = Fabric.from_obj(name, path, resolved, self.solver, translation, rotation)
        self.add_fabric(fabric)
        
        return fabric
    
    @staticmethod
    def _resolve_material(material) -> Material:
        if material is None:
            return Material.from_preset("cotton")
        elif isinstance(material, str):
            return Material.from_preset(material)
        elif isinstance(material, dict):
            return Material.from_dict(material)
        elif isinstance(material, Material):
            return material
        else:
            raise TypeError(
                f"Invalid material type: {type(material).__name__}. "
                f"Expected str, dict, Material or None."
            )

    def add_floor(self, name: str, height: float = 0.0, friction: float =0.5):
        """
        Adds a horizontal floor to the simulation.

        Args:
            name: Name of the floor collider.
            height: Vertical position of the floor in world space.
            friction: Surface friction in the range [0.0, 1.0].
                    0.0 is completely slippery, 1.0 is fully grippy.
        """
        self.world.add_plane_collider([0.0, float(height), 0.0], [0.0, 1.0, 0.0], float(friction))
        sdk.Logger.info(f"Added collision floor '{name}' at Y={height}")
        self._colliders[name] = len(self.world.get_colliders()) - 1

    def add_sphere(self, name: str, center: np.array, radius: float, friction: float = 0.5):
        self.world.add_sphere_collider(center, float(radius), float(friction))
        sdk.Logger.info(f"Added sphere collider '{name}' at {center}")
        self._colliders[name] = len(self.world.get_colliders()) - 1

    def add_capsule(self, name: str, start: np.array, end: np.array, radius: float = 1.0, friction: float = 0.5):
        self.world.add_capsule_collider(start, end, float(radius), float(friction))
        sdk.Logger.info(f"Added capsule collider '{name}' from {start} to {end}")
        self._colliders[name] = len(self.world.get_colliders()) - 1
        
    
    def add_mesh(self, name: str, path: str, friction: float = 0.5):
        self.world.add_mesh_collider(path, friction)
        self._colliders[name] = len(self.world.get_colliders()) - 1

    def step(self, dt: float = 1/60):
        self._last_dt = dt
        if self._frame_counter in self.actions:
            for action in self.actions[self._frame_counter]:
                action(self)
        self.solver.update(self.world, dt)
        if self._recording:
            self._ke_history = np.append(self._ke_history, self.kinetic_energy(dt))
            self._pe_history = np.append(self._pe_history, self.potential_energy(dt))
        self._frame_counter += 1
        
    def get_positions(self) -> np.ndarray:
        particles = self.solver.get_particles()
        return np.array([p.get_position() for p in particles], dtype=np.float64)
    
    def reset(self):
        """Clears the simulation, removing all fabrics, colliders and forces."""
        self.world.clear()
        self.solver.clear()
        self.cloth_objects = {}
        self._aero_forces = {}
        sdk.Logger.info("Simulation world reset.")
        
    def soft_reset(self):
        self.solver.soft_reset()
        
    def bake_alembic(self, filepath: str, start_frame: int = 0, end_frame: int = 120, fps: float = 60.0) -> bool:
        """
        Bakes the simulation to an Alembic (.abc) cache file.

        Advances the simulation from start_frame to end_frame and writes
        each frame to disk. The simulation state is modified in place.

        Args:
            filepath:    Output path for the .abc file. Created or overwritten if it exists.
            start_frame: First frame to bake.
            end_frame:   Last frame to bake.
            fps:         Frames per second, used to compute the timestep.

        Returns:
            True if the file was written successfully, False otherwise.
        """
        
        if not self.cloth_objects:
            raise RuntimeError("No cloth objects found in simulation to bake.")

        exporter = sdk.AlembicExporter()
        dt = 1.0 / fps
        
        names = list(self.cloth_objects.keys())
        global_indices_list = []
        particle_indices_list = []
        for name in names:
            cloth_obj = self.cloth_objects[name]
            global_indices_list.append(cloth_obj.get_triangles())
            particle_ids = cloth_obj.instance.get_particle_indices()
            particle_indices_list.append(np.array(particle_ids, dtype=np.int32))

        sdk.Logger.info(f"Baking simulation to {filepath}...")
        
        if not exporter.open(filepath, names, self.get_positions(),
                             global_indices_list, particle_indices_list):
            sdk.Logger.error(f"Failed to create Alembic file: {filepath}")
            return False

        total_frames = end_frame - start_frame
        
        for frame_idx in tqdm(range(total_frames), desc="Baking Alembic", unit="frames"):
            self.step(dt)
            current_time = frame_idx * dt
            exporter.write_frame(self.get_positions(), current_time)

        self.actions.clear()
        exporter.close()
        sdk.Logger.info(f"Bake completed successfully: {filepath}")
        return True
    
    def save_snapshot(self, filename: str, fabric_name: str):
        if fabric_name not in self.cloth_objects:
            raise RuntimeError(f"Fabric '{fabric_name}' not found.")
            
        fabric = self.cloth_objects[fabric_name]
        
        sdk.OBJExporter.export_obj(filename, fabric.instance, self.solver)
        sdk.Logger.info(f"Snapshot saved: {filename}")
        return True
    
    def save_scene(self, path: str = "data/configs/scenes/default.json", name: str = "default"):
        sdk.SceneExporter.save_scene(path, name, self.solver, self.world)
    
    def view(self, width: int = 1280, height: int = 720, title: str = "Tissu | Live Simulation"):
        if not self.cloth_objects:
            sdk.Logger.warn("No cloth objects to visualize.")
            
        if self.app is None:
            self.app = sdk.Application()
            
        current_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.dirname(os.path.dirname(current_dir))
        shader_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "shaders", "")
        
        self.app.set_solver(self.solver)
        self.app.set_world(self.world)
        
        if self.cloth_objects:
            first_cloth_name = list(self.cloth_objects.keys())[0]
            fabric = self.cloth_objects[first_cloth_name]
            self.app.set_cloth(fabric.instance)
            
        sdk.Logger.info(f"Initializing OpenGL Viewer")
        sdk.Logger.info(f"Shader Path : {shader_path}")
        
        if self._aero_forces:
            first = list(self._aero_forces.values())[0]
            self.app.set_aero_force(first)
        
        if not self.app.init(width, height, title, shader_path):
            sdk.Logger.error("Failed to initialize the viewer.")
            return
        
        self.app.sync_visual_topology()
        sdk.Logger.info("Starting simulation loop.")
        self.app.run()

        self.app.shutdown()
        sdk.Logger.info("Viewer closed.")
        
        
    def plot(self, fabric_name: str | None = None) -> None:
        if fabric_name is not None:
            fabrics = [self.get_fabric(fabric_name)]
        else:
            fabrics = list(self.cloth_objects.values())

        if not fabrics:
            sdk.Logger.warn("No fabrics to plot.")
            return

        fig = plt.figure()
        ax = fig.add_subplot(projection="3d")

        all_x, all_y, all_z = [], [], []
        for fabric in fabrics:
            positions = fabric.get_positions()
            triangles_global = fabric.get_triangles().reshape(-1, 3)
            local_ids = fabric.instance.get_particle_indices()
            mapper = np.full(max(local_ids) + 1, -1, dtype=np.int32)
            mapper[local_ids] = np.arange(len(local_ids))
            triangles = mapper[triangles_global]
            x, y, z = positions[:, 0], positions[:, 1], positions[:, 2]
            all_x.append(x)
            all_y.append(y)
            all_z.append(z)

            ax.plot_trisurf(
                x, z, y,
                triangles=triangles,
                cmap="plasma",
                vmin=float(y.min()),
                vmax=float(y.max()),
                edgecolor="none"
            )

        all_x = np.concatenate(all_x)
        all_y = np.concatenate(all_y)
        all_z = np.concatenate(all_z)

        max_range = np.array([
            all_x.max() - all_x.min(),
            all_y.max() - all_y.min(),
            all_z.max() - all_z.min()
        ]).max() / 2.0

        mid_x = (all_x.max() + all_x.min()) / 2
        mid_y = (all_y.max() + all_y.min()) / 2
        mid_z = (all_z.max() + all_z.min()) / 2

        ax.set_xlim(mid_x - max_range, mid_x + max_range)
        ax.set_ylim(mid_z - max_range, mid_z + max_range)
        ax.set_zlim(mid_y - max_range, mid_y + max_range)

        ax.set_xlabel("X")
        ax.set_ylabel("Z")
        ax.set_zlabel("Y (height)")
        plt.tight_layout()
        plt.show()


    def plot_gif(self, fabric_name: str | None = None, start: int = 0, end: int = 120, fps: int = 30, filename: str = "simulation.gif") -> None:
        if fabric_name is not None:
            fabrics = [self.get_fabric(fabric_name)]
        else:
            fabrics = list(self.cloth_objects.values())

        if not fabrics:
            sdk.Logger.warn("No fabrics to plot.")
            return

        init_pos = np.concatenate([f.get_positions() for f in fabrics])
        x0, y0, z0 = init_pos[:, 0], init_pos[:, 1], init_pos[:, 2]
        max_range = np.array([x0.max()-x0.min(), y0.max()-y0.min(), z0.max()-z0.min()]).max() / 2.0
        mid_x, mid_y, mid_z = (x0.max()+x0.min())/2, (y0.max()+y0.min())/2, (z0.max()+z0.min())/2

        fig = plt.figure()
        ax = fig.add_subplot(projection="3d")

        sdk.Logger.info(f"Rendering GIF '{filename}' ({end - start} frames @ {fps} fps)...")

        with imageio.get_writer(filename, mode="I", fps=fps) as writer:
            for _ in tqdm(range(end - start), desc="Rendering GIF", unit="frame"):
                self.step(self._last_dt)

                ax.clear()
                for fabric in fabrics:
                    positions = fabric.get_positions()
                    triangles_global = fabric.get_triangles().reshape(-1, 3)
                    local_ids = fabric.instance.get_particle_indices()
                    mapper = np.full(max(local_ids) + 1, -1, dtype=np.int32)
                    mapper[local_ids] = np.arange(len(local_ids))
                    triangles = mapper[triangles_global]
                    x, y, z = positions[:, 0], positions[:, 1], positions[:, 2]

                    ax.plot_trisurf(x, z, y, triangles=triangles, cmap="plasma",
                                    vmin=float(y.min()), vmax=float(y.max()), edgecolor="none")
                ax.set_xlim(mid_x - max_range, mid_x + max_range)
                ax.set_ylim(mid_z - max_range, mid_z + max_range)
                ax.set_zlim(mid_y - max_range, mid_y + max_range)
                ax.set_xlabel("X")
                ax.set_ylabel("Z")
                ax.set_zlabel("Y (height)")
                ax.set_title(f"Frame {self.frame}")

                fig.canvas.draw()
                image = np.frombuffer(fig.canvas.buffer_rgba(), dtype=np.uint8)
                image = image.reshape(fig.canvas.get_width_height()[::-1] + (4,))
                image = image[:, :, :3]
                writer.append_data(image)

        plt.close(fig)
        sdk.Logger.info(f"GIF saved: {filename}")


    def plot_energy(self) -> None:
        frames = np.arange(len(self._ke_history))
        fig, ax1 = plt.subplots()
        ax1.plot(frames, self._ke_history, label="KE", color="tab:blue")
        ax1.plot(frames, self._ke_history + self._pe_history, label="Total", color="tab:green")
        ax1.set_xlabel("Frame")
        ax1.set_ylabel("Kinetic / Total Energy")
        ax1.tick_params(axis="y")
        ax2 = ax1.twinx()
        ax2.plot(frames, self._pe_history, label="PE", color="tab:orange")
        ax2.set_ylabel("Potential Energy")
        ax2.tick_params(axis="y")
        fig.legend(loc="upper right")
        plt.show()


    def load_material(self, filepath: str, cloth_name: str) -> None:
        """
        Loads material properties from a JSON file and applies them to a fabric.

        Args:
            filepath:   Path to the material JSON file.
            cloth_name: Name of the target fabric.
        """
        if cloth_name not in self.cloth_objects:
            raise KeyError(f"Fabric '{cloth_name}' not found in simulation.")
        
        fabric = self.cloth_objects[cloth_name]
        sdk.ConfigLoader.load_material(filepath, fabric.instance.get_material())

    def load_physics(self, filepath: str) -> None:
        """
        Loads physics parameters from a JSON file and applies them to the simulation.

        Args:
            filepath: Path to the physics JSON file.
        """
        sdk.ConfigLoader.load_physics(filepath, self.solver, self.world)

    def save_material(self, filepath: str, cloth_name: str) -> None:
        """
        Saves the material properties of a fabric to a JSON file.

        Args:
            filepath:   Destination path for the material JSON file.
            cloth_name: Name of the fabric whose material will be saved.
        """
        if cloth_name not in self.cloth_objects:
            raise KeyError(f"Fabric '{cloth_name}' not found in simulation.")
        
        mat = self.cloth_objects[cloth_name].instance.get_material()
        sdk.ConfigLoader.save_material(filepath, mat, cloth_name)

    def save_physics(self, filepath: str, name: str = "physics") -> None:
        """
        Saves the current physics parameters to a JSON file.

        Args:
            filepath: Destination path for the physics JSON file.
            name:     Identifier written into the file.
        """
        sdk.ConfigLoader.save_physics(filepath, self.solver, self.world, name)
        
    def save_state(self, filepath: str = "default.tissu"):
        sdk.StateSerializer.save(filepath, self.solver, self.world)
        
    def load_state(self, filepath: str):
        sdk.StateSerializer.load(filepath, self.solver, self.world)
        
    @staticmethod
    def get_state_status(filepath: str):
        return sdk.StateSerializer.get_state_info(filepath)
    
    @staticmethod
    def get_scene_status(filepath: str):
        return sdk.SceneLoader.get_scene_header(filepath)
        
    def move_collider(self, name: str, new_position: np.array, new_rotation: np.array = None) -> None:
        if new_rotation is None:
            new_rotation = np.array([0.0, 0.0, 0.0, 1.0])
        
        index = self._colliders.get(name)
        if index is None:
            raise KeyError(f"Collider '{name}' not found in simulation.")
        
        self.world.move_collider(index, new_position, new_rotation)
        
    def __repr__(self) -> str:
        fabric_list = ", ".join(repr(fabric) for fabric in self.cloth_objects.values())
    
        return (
            f"Simulation(substeps={self.substeps}, "
            f"iterations={self.iterations}, "
            f"gravity={self.gravity}, "
            f"wind={self.wind})\n"
            f"Fabrics: {fabric_list if fabric_list else 'none'}\n"
        )
        
    def __getitem__(self, key) -> Fabric:
        return self.cloth_objects[key]

    def __len__(self) -> int:
        return len(self.cloth_objects)

    def __iter__(self) -> Iterator[Fabric]:
        return iter(self.cloth_objects.values())

    def __contains__(self, name: str) -> bool:
        return name in self.cloth_objects
    
    def __enter__(self):
        return self
    
    def __exit__(self, exc_type, exc, tb):
        self.reset()

class Fabric:
    def __init__(self, name: str, material: Material):
        self.name = name
        self.material = material 
        self.instance = sdk.Cloth(name, material.native)
        self._solver = None
        self._rows = 0
        self._cols = 0
        self._pins = np.empty(0, dtype=int)

    @classmethod
    def grid(cls, name: str, rows: int, cols: int, spacing: float, material: Material, solver: sdk.Solver, translation=(0.0, 0.0, 0.0), rotation=(0.0, 0.0, 0.0, 1.0)) -> Fabric:
        fabric = cls(name, material)
        fabric._rows = rows
        fabric._cols = cols
        
        factory = sdk.ClothMesh()
        factory.init_grid(rows, cols, spacing, fabric.instance, solver, translation, rotation)
        return fabric

    @classmethod
    def from_obj(cls, name: str, path: str, material: Material, solver: sdk.Solver, translation=(0.0, 0.0, 0.0), rotation=(0.0, 0.0, 0.0, 1.0)):
        fabric = cls(name, material)
        success, pos, indices = sdk.OBJLoader.load(path)
        if not success:
            raise FileNotFoundError(f"Could not load OBJ: {path}")
            
        factory = sdk.ClothMesh()
        factory.build_from_mesh(pos, indices, fabric.instance, solver, path, translation, rotation)
        return fabric
    
    def update_material(self, density: float = None, structural: float = None, shear: float = None, bending: float = None):
        current_mat = self.instance.get_material()
        
        if density is not None: current_mat.density = float(density)
        if structural is not None: current_mat.structural_compliance = float(structural)
        if shear is not None: current_mat.shear_compliance = float(shear)
        if bending is not None: current_mat.bending_compliance = float(bending)
    
        self.instance.set_material(current_mat)
        sdk.Logger.info(f"Updated material for '{self.name}'")
        
    def enable_volume_preservation(self, compliance=1e-4) -> float:
        """
        Adds internal pressure to a closed mesh to maintain it's volume.
        For instance, this function could be used to simulate a pillow or puffy jackets.
        
        Args: 
            compliance: Inverse of stiffness. Lower values make the volume rigid
            while higher values allow more compression.
            
        Returns:
            float: The calculated rest volume of the mesh.
            
        Raises:
            RuntimeError: If the fabric isn't in a simulation or the mesh isn't closed.

        """
        if self._solver is None:
            raise RuntimeError("Fabric must be added to a Simulation before enabling volume preservation.")
        
        if not self.instance.is_closed():
            raise RuntimeError(f"Fabric '{self.name}' is not a closed mesh.")
        
        rest_volume = self._solver.add_volume_constraint(
            self.instance.get_triangles_native(),
            self._solver.get_particles(),
            compliance
        )
        
        self.instance.set_rest_volume(rest_volume)
        return rest_volume
    
    @property
    def pins(self):
        return self._pins

    def get_positions(self) -> np.ndarray:
        """
        Returns the particle's positions of the Fabric.
        """
        
        all_particles = self._solver.get_particles()
        my_indices = self.instance.get_particle_indices()
        return np.array([all_particles[idx].get_position() for idx in my_indices], dtype=np.float64)
    
    def upper_particles(self, threshold: float = 0.01):
        pos = self.get_positions()
        if len(pos) == 0:
            return np.empty(0, dtype=int)
            
        max_y = np.max(pos[:, 1])
        mask = pos[:, 1] >= (max_y - threshold)
        return np.where(mask)[0]
    
    def lower_particles(self, threshold: float = 0.01):
        pos = self.get_positions()
        if len(pos) == 0:
            return np.empty(0, dtype=int)
            
        min_y = np.min(pos[:, 1])
        mask = pos[:, 1] <= (min_y + threshold)
        return np.where(mask)[0]

    def pin_by_height(self, threshold: float = 0.01, compliance: float = 0.0):
        if self._solver is None:
            raise RuntimeError("Fabric must be added to a Simulation before pinning.")
        
        pos = self.get_positions()
        my_ids = self.instance.get_particle_indices()
        self._pins = self.upper_particles(threshold)
        
        if len(self._pins) == 0:
            sdk.Logger.warn(f"Fabric '{self.name}': No particles found at top to pin.")
            return

        for idx in self._pins:
            global_id = my_ids[idx]
            target_pos = pos[idx]
            self._solver.add_pin(global_id, target_pos, compliance)
        
        self.instance.set_pin(sdk.Pin(sdk.PinMode.BY_HEIGHT, compliance, threshold))
        sdk.Logger.info(f"Fabric '{self.name}': Pinned {len(self._pins)} vertices by height.")
        
    def pin_top_corners(self, threshold: float = 0.01, compliance: float = 0.0):
        if self._solver is None:
            raise RuntimeError("Fabric must be added to a Simulation before pinning.")
        
        pos = self.get_positions()
        my_ids = self.instance.get_particle_indices()
        top_indices = self.upper_particles(threshold)
        if len(top_indices) == 0:
            sdk.Logger.warn(f"Fabric '{self.name}': No particles found at top to pin.")
            return

        top_x_coords = pos[top_indices, 0]
        local_min_idx = np.argmin(top_x_coords) 
        local_max_idx = np.argmax(top_x_coords)
        idx_left = top_indices[local_min_idx]
        idx_right = top_indices[local_max_idx]
        self._pins = np.array([idx_left, idx_right])
        
        for idx in self._pins:
            global_id = my_ids[idx]
            target_pos = pos[idx]
            self._solver.add_pin(global_id, target_pos, compliance)
            
        self.instance.set_pin(sdk.Pin(sdk.PinMode.TOP_CORNERS, compliance, threshold))
        sdk.Logger.info(f"Fabric '{self.name}': Pinned top corners (IDs: {list(self._pins)})")
        
    def unpin(self):
        my_ids = self.instance.get_particle_indices()
        
        for idx in self._pins:
            global_id = my_ids[idx]
            self._solver.unpin(global_id)
            
        self._pins = np.empty(0)
        sdk.Logger.info(f"Fabric '{self.name}': Unpinned ")
        
    def get_particle_id(self, row: int, col: int):
        return self.instance.get_particle_id(row, col)
    
    def get_triangles(self):
        return np.array(self.instance.get_triangles())
    
    def get_pins(self):
        return self._pins
    
    def particle_count(self) -> int:
        return len(self.get_positions())
    
    def is_pinned(self) -> bool:
        return len(self._pins) != 0
    
    def __repr__(self):
        return f"('{self.name}', {self.particle_count()} particles)"
    
class Material:
    _BUILTIN_PRESETS = {
        "silk":    (0.1,  1e-9,  1e-8,  0.1),
        "cotton":  (0.2,  1e-9,  1e-8,  0.01),
        "denim":   (0.45, 1e-10, 1e-9,  0.0005),
        "leather": (0.7,  0.0,   1e-10, 1e-6),
        "spandex": (0.15, 0.005, 0.005, 0.1),
    }

    def __init__(self, density=0.1, structural=1e-9, shear=1e-8, bending=0.01):
        if density <= 0:
            raise ValueError("Density must be greater than 0.")
        if any(v < 0 for v in [structural, shear, bending]):
            raise ValueError("Compliance values must be >= 0.")

        self._native = sdk.ClothMaterial(
            float(density),
            float(structural),
            float(shear),
            float(bending)
        )

    @classmethod
    def from_preset(cls, name: str, presets_path: str = None) -> "Material":
        """
        Creates a Material from a named preset.
        
        Tries to load from a JSON file if presets_path is provided.
        Falls back to built-in presets if the file is not found.

        Args:
            name:         Preset name. Built-in options: silk, cotton, denim, leather, spandex.
            presets_path: Optional path to the materials directory.

        Raises:
            ValueError: If the preset name is not found in built-in presets.
        """
        if presets_path:
            filepath = os.path.join(presets_path, f"{name}.json")
            if os.path.exists(filepath):
                try:
                    mat = cls()
                    sdk.ConfigLoader.load_material(filepath, mat._native)
                    return mat
                except Exception as e:
                    sdk.Logger.warn(f"Could not load preset from {filepath}: {e}. Falling back to built-in.")

        if name not in cls._BUILTIN_PRESETS:
            raise ValueError(
                f"Unknown preset: '{name}'. "
                f"Available: {list(cls._BUILTIN_PRESETS.keys())}"
            )

        return cls(*cls._BUILTIN_PRESETS[name])

    @classmethod
    def from_dict(cls, data: dict) -> "Material":
        """
        Creates a Material from a dictionary of properties.

        Args:
            data: Dictionary with keys: density, structural_compliance,
                  shear_compliance, bending_compliance.
        """
        return cls(
            density=data.get("density", 0.1),
            structural=data.get("structural_compliance", 1e-9),
            shear=data.get("shear_compliance", 1e-8),
            bending=data.get("bending_compliance", 0.01)
        )

    @property
    def density(self): return self._native.density
    @density.setter
    def density(self, v):
        if float(v) <= 0:
            raise ValueError("Density must be greater than 0.")
        self._native.density = float(v)

    @property
    def structural(self): return self._native.structural_compliance
    @structural.setter
    def structural(self, v):
        if float(v) < 0:
            raise ValueError("Structural compliance must be >= 0.")
        self._native.structural_compliance = float(v)

    @property
    def shear(self): return self._native.shear_compliance
    @shear.setter
    def shear(self, v):
        if float(v) < 0:
            raise ValueError("Shear compliance must be >= 0.")
        self._native.shear_compliance = float(v)

    @property
    def bending(self): return self._native.bending_compliance
    @bending.setter
    def bending(self, v):
        if float(v) < 0:
            raise ValueError("Bending compliance must be >= 0.")
        self._native.bending_compliance = float(v)

    @property
    def native(self) -> "sdk.ClothMaterial":
        """Returns the underlying C++ ClothMaterial."""
        return self._native

    def __repr__(self):
        return (
            f"Material(density={self.density}, structural={self.structural}, "
            f"shear={self.shear}, bending={self.bending})"
        )
