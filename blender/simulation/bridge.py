from __future__ import annotations
from pathlib import Path
import numpy as np

import bpy

ADDON_PATH = Path(__file__).resolve().parents[1]
LIBS_PATH = str(ADDON_PATH / "libs")
if LIBS_PATH not in __import__("sys").path:
    __import__("sys").path.insert(0, LIBS_PATH)

from _cloth_sdk_core import (
    World, Solver, Cloth, ClothMesh, ClothMaterial,
    GravityForce, AerodynamicForce, Pin, PinMode,
    MeshCollider, PlaneCollider, SphereCollider, CapsuleCollider,
    AlembicExporter, OBJLoader, SceneExporter, SceneLoader,
    ConfigLoader, Logger,
)

class Simulation:
    def __init__(self, substeps: int = 10, iterations: int = 2, gravity: float = -9.81, thickness: float = 0.02):
        self.world = World()
        self.solver = Solver()
        self._substeps = substeps
        self._iterations = iterations
        self._thickness = thickness
        self._gravity = gravity
        self._wind = [0.0, 0.0, 0.0]
        self._air_thickness = 0.01
    
    @property
    def substeps(self):
        return self._substeps
    
    @substeps.setter
    def substeps(self, value):
        print(f"--> [Bridge] Change substeps to: {value}")
        self._substeps = value
    
    @property
    def iterations(self):
        return self._iterations
    
    @iterations.setter
    def iterations(self, value):
        print(f"--> [Bridge] Change iterations to: {value}")
        self._iterations = value

    @property
    def thickness(self):
        return self._thickness
    
    @thickness.setter
    def thickness(self, value):
        print(f"--> [Bridge] Change thickness to: {value}")
        self._thickness = value
        
    @property
    def gravity(self):
        return self._gravity
    
    @gravity.setter
    def gravity(self, value):
        print(f"--> [Bridge] Change gravity to: {value}")
        self._gravity = value
    
    @property
    def wind(self):
        return self._wind
    
    @wind.setter
    def wind(self, value):
        print(f"--> [Bridge] Change wind to: {value}")
        self._wind = np.array(value, dtype=np.float32)
        
    @property
    def air_thickness(self):
        return self._air_thickness
    
    @air_thickness.setter
    def air_thickness(self, value):
        print(f"--> [Bridge] Change air thickness to: {value}")
        self._air_thickness = value

tissu_sim_instance = None

def get_simulation() -> Simulation:
    global tissu_sim_instance
    if tissu_sim_instance is None:
        tissu_sim_instance = Simulation()
    return tissu_sim_instance

classes = [Simulation]

def register():
    global tissu_sim_instance
    tissue_sim_instance = Simulation()
    tissu_sim_instance = tissue_sim_instance
    Logger.info("Tissu simulation bridge registered.")


def unregister():
    global tissu_sim_instance
    if tissu_sim_instance is not None:
        tissu_sim_instance = None
    Logger.info("Tissu simulation bridge unregistered.")

