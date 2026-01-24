from python.cloth_sdk import Simulation, Fabric
import os

def falling():
    sim = Simulation(
        substeps=10,
        iterations=2,
        gravity=-9.81,
        thickness=0.05
    )

    sim.wind = [2.0, 0.0, 8.0]
    sim.air_density = 0.1

    sim.add_floor(height=0.0, friction=0.5)

    material = {
        "density": 0.1,
        "structural_compliance": 1e-9,
        "shear_compliance": 1e-8,
        "bending_compliance": 0.1,
    }

    curtain = Fabric.grid(
        name="Debug_Curtain",
        rows=120,
        cols=80,
        spacing=0.1,
        material=material,
        solver=sim.solver
    )

    sim.add_fabric(curtain)

    curtain.pin_by_height(
        solver=sim.solver,
        threshold=0.01,
        compliance=0.001
    )

    sim.view()

if __name__ == "__main__":
    falling()
