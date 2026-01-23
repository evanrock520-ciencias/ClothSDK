from _backend import load
load()

import cloth_sdk as sdk
import numpy as np

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

        aero = sdk.AerodynamicForce(fabric.instance.get_aerofaces(), self.wind, self.air_density)
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