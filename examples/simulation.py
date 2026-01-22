import os
import sys
import time
from load_path import load

load()

try:
    import cloth_sdk as sdk
except ImportError as e:
    print(f"Error: {e}")
    sys.exit(1)

def falling():
    solver = sdk.Solver()
    
    gravity = sdk.GravityForce([0.0, -9.81, 0.0])
    solver.add_force(gravity)
    
    solver.set_substeps(10)
    solver.set_iterations(2)
    solver.set_thickness(0.05)

    mat = sdk.ClothMaterial(0.2, 1e-10, 1e-8, 0.01)
    curtain = sdk.Cloth("Debug_Curtain", mat)
    factory = sdk.ClothMesh()

    rows, cols = 120, 80
    spacing = 0.1
    sdk.Logger.info(f"Weaving {rows}x{cols} mesh...")
    
    factory.init_grid(rows, cols, spacing, curtain, solver)

    aero_faces = curtain.get_aerofaces()
    wind_force = sdk.AerodynamicForce(aero_faces, [2.0, 0.0, 8.0], 0.1)
    solver.add_force(wind_force)

    particles = solver.get_particles()
    top_row = rows - 1

    p_id = curtain.get_particle_id(top_row, 0)
    target = particles[p_id].get_position()
    solver.add_pin(p_id, target, 0.001) 

    exporter = sdk.AlembicExporter()
    output_path = "data/animations/falling.abc"

    initial_pos = [p.get_position() for p in particles]
    indices = curtain.get_triangles()

    if not exporter.open(output_path, initial_pos, indices):
        sdk.Logger.error("Failed to create Alembic file!")
        return

    frames = 600
    dt = 0.016

    sdk.Logger.info(f"Simulating {frames} frames to {output_path}...")

    for f in range(frames):
        solver.update(dt)

        current_pos = [p.get_position() for p in solver.get_particles()]
        exporter.write_frame(current_pos, f * dt)

        if f % 20 == 0:
            print(f"      Writing frame {f}...")

    exporter.close()
    sdk.Logger.info("Done! Physics export finished.")
    print(f"Check the result in Blender: {os.path.abspath(output_path)}")

if __name__ == "__main__":
    falling()