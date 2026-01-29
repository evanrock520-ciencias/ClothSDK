from python.cloth_sdk import Simulation, Fabric, Material
import os

def falling():
    sim = Simulation(
        substeps=15,
        iterations=2,
        gravity=-9.81,
        thickness=0.05
    )

    sim.wind = [0.0, 0.0, 0.0]
    sim.air_density = 0.1

    sim.add_floor(height=0.0, friction=0.5)

    material = {
        "density": 0.1,
        "structural_compliance": 1e-9,
        "shear_compliance": 1e-8,
        "bending_compliance": 0.1
    }

    curtain = Fabric.grid(
        name="Curtain",
        rows=100,
        cols=100,
        spacing=0.05,
        material=material,
        solver=sim.solver
    )

    sim.add_fabric(curtain)
    Material.apply_preset(curtain, "silk")

    sim.bake_alembic(
        filepath="data/animations/curtain.abc",
        start_frame=0,
        end_frame=96,
        fps=24
    )

if __name__ == "__main__":
    falling()
