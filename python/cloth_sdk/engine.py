import _cloth_sdk_core as sdk
import numpy as np
import os


class Simulation:
    def __init__(self, substeps=10, iterations=2, gravity=-9.81, thickness=0.02):
        self.world = sdk.World()
        self.solver = sdk.Solver()
        
        self._gravity_vector = np.array([0.0, float(gravity), 0.0], dtype=np.float64)
        self._gravity_force = sdk.GravityForce(self._gravity_vector)
        self.world.add_force(self._gravity_force)
        
        self.cloth_objects = {}  
        self._aero_forces = {}   
        
        self.substeps = substeps
        self.iterations = iterations
        self.gravity = gravity
        self.thickness = thickness
        self.wind = [0.0, 0.0, 0.0]
        self.air_density = 0.1
        
        self.app = sdk.Application()

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
        

    def add_fabric(self, fabric):
        if fabric.name in self.cloth_objects:
            sdk.Logger.warn(f"Fabric '{fabric.name}' already exists. Overwriting.")

        self.world.add_cloth(fabric.instance)

        aero = sdk.AerodynamicForce(
            fabric.instance.get_aerofaces(), 
            self.wind, 
            self.air_density
        )
        self._aero_forces[fabric.name] = aero
        self.world.add_force(aero)

        self.cloth_objects[fabric.name] = fabric
        sdk.Logger.info(f"Successfully added fabric: {fabric.name}")

    def add_floor(self, height=0.0, friction=0.5):
        self.world.add_plane_collider([0.0, float(height), 0.0], [0.0, 1.0, 0.0], float(friction))
        sdk.Logger.info(f"Added collision floor at Y={height}")

    def add_sphere(self, name, center, radius, friction=0.5):
        self.world.add_sphere_collider(center, float(radius), float(friction))
        sdk.Logger.info(f"Added sphere collider '{name}' at {center}")
    
    def step(self, dt=1.0/60.0):
        self.solver.update(self.world, dt)
        
    def get_positions(self) -> np.ndarray:
        particles = self.solver.get_particles()
        return np.array([p.get_position() for p in particles], dtype=np.float64)
    
    def reset(self):
        self.world.clear()
        self.solver.clear()
        self.cloth_objects = {}
        self._aero_forces = {}
        sdk.Logger.info("Simulation world reset.")
        
    def bake_alembic(self, filepath, start_frame=0, end_frame=120, fps=24.0):
        if not self.cloth_objects:
            sdk.Logger.error("No cloth objects found in simulation to bake.")
            return False

        exporter = sdk.AlembicExporter()
        dt = 1.0 / fps
        
        first_cloth_name = list(self.cloth_objects.keys())[0]
        cloth_obj = self.cloth_objects[first_cloth_name]
        
        indices = cloth_obj.get_triangles() 
        particles = self.solver.get_particles()
        initial_pos = [p.get_position() for p in particles]

        sdk.Logger.info(f"Baking simulation to {filepath}...")
        
        if not exporter.open(filepath, initial_pos, indices):
            sdk.Logger.error(f"Failed to create Alembic file: {filepath}")
            return False

        total_frames = end_frame - start_frame
        
        for frame_idx in range(total_frames):
            self.step(dt)
            
            current_pos = [p.get_position() for p in self.solver.get_particles()]
            
            current_time = frame_idx * dt
            exporter.write_frame(current_pos, current_time)
            
            if frame_idx % (max(1, total_frames // 10)) == 0:
                sdk.Logger.info(f"   Bake progress: {int((frame_idx/total_frames)*100)}%")

        exporter.close()
        sdk.Logger.info(f"Bake completed successfully: {filepath}")
        return True
    
    def view(self, width=1280, height=720, title="ClothSDK | Live Simulation"):
        if not self.cloth_objects:
            sdk.Logger.warn("No cloth objects to visualize.")
            
        current_dir = os.path.dirname(os.path.abspath(__file__))
        project_root = os.path.dirname(os.path.dirname(current_dir))
        shader_path = os.path.join(project_root, "viewer", "shaders", "")
        
        self.app.set_solver(self.solver)
        
        if self.cloth_objects:
            first_cloth_name = list(self.cloth_objects.keys())[0]
            fabric = self.cloth_objects[first_cloth_name]
            self.app.set_cloth(fabric.instance)
            
        sdk.Logger.info(f"Initializing OpenGL Viewer")
        sdk.Logger.info(f"Shader Path : {shader_path}")
        
        if not self.app.init(width, height, title, shader_path):
            sdk.Logger.error("Failed to initialize the viewer.")
            return
        
        self.app.sync_visual_topology()
        sdk.Logger.info("Starting simulation loop.")
        self.app.run()

        self.app.shutdown()
        sdk.Logger.info("Viewer closed.")
        
class Fabric:
    def __init__(self, name, material):
        self.name = name
        self.material = material 
        
        self._material_instance = sdk.ClothMaterial(
            float(material["density"]),
            float(material["structural_compliance"]),
            float(material["shear_compliance"]),
            float(material["bending_compliance"]),
        
        )
        
        self.instance = sdk.Cloth(name, self._material_instance)
        self._rows = 0
        self._cols = 0

    @classmethod
    def grid(cls, name, rows, cols, spacing, material, solver):
        fabric = cls(name, material)
        fabric._rows = rows
        fabric._cols = cols
        
        factory = sdk.ClothMesh()
        factory.init_grid(rows, cols, spacing, fabric.instance, solver)
        return fabric

    @classmethod
    def from_obj(cls, name, path, material, solver):
        fabric = cls(name, material)
        success, pos, indices = sdk.OBJLoader.load(path)
        if not success:
            raise FileNotFoundError(f"Could not load OBJ: {path}")
            
        factory = sdk.ClothMesh()
        factory.build_from_mesh(pos, indices, fabric.instance, solver)
        return fabric

    def get_positions(self, solver):
        all_particles = solver.get_particles()
        my_indices = self.instance.get_particle_indices()
        
        pos_list = [all_particles[idx].get_position() for idx in my_indices]
        return np.array(pos_list, dtype=np.float64)

    def pin_by_height(self, solver, threshold=0.01, compliance=0.0):
        pos = self.get_positions(solver)
        my_ids = self.instance.get_particle_indices()
        
        max_y = np.max(pos[:, 1])
        
        mask = pos[:, 1] >= (max_y - threshold)
        indices_to_pin = np.where(mask)[0]
        
        for idx in indices_to_pin:
            global_id = my_ids[idx]
            target_pos = pos[idx]
            solver.add_pin(global_id, target_pos, compliance)
            
        sdk.Logger.info(f"Fabric '{self.name}': Pinned {len(indices_to_pin)} vertices by height.")

    def get_particle_id(self, row, col):
        return self.instance.get_particle_id(row, col)
    
    def get_triangles(self):
        return self.instance.get_triangles()
