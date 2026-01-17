import os
import sys
from load_path import load

load()

try:
    import cloth_sdk as sdk
    import numpy as np
except ImportError as e:
    print(f"Error importing modules: {e}")
    sys.exit(1)

def run_curtain_simulation():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    shader_path = os.path.join(project_root, "viewer", "shaders", "")
    
    sdk.Logger.info("========================================")
    print(f"[Python] Project Root: {project_root}")
    print(f"[Python] Shader Path:  {shader_path}")
    sdk.Logger.info("========================================")

    solver = sdk.Solver()
    mesh = sdk.ClothMesh()

    solver.set_gravity([0.0, -9.81, 0.0])
    solver.set_substeps(5)
    solver.set_iterations(20) 
    
    solver.set_wind([6.0, 0.0, 10.0]) 
    solver.set_air_density(0.0)
    
    dt = 1 / 60
    solver.update(dt)

    rows, cols = 40, 40
    spacing = 0.2
    
    mesh.set_material(0.2, 0.0, 1e-6, 0.05)
    
    sdk.Logger.info(f"Weaving {rows}x{cols} curtain grid...")
    mesh.init_grid(rows, cols, spacing, solver)

    top_row = rows - 1
    for c in range(cols):
        p_id = mesh.get_particle_id(top_row, c)
        solver.set_particle_inverse_mass(p_id, 0.0)
    
    sdk.Logger.info(f"Pinned {cols} vertices to the top rail.")

    app = sdk.Application()
    
    if not app.init(1280, 720, "ClothSDK | XPBD Real-time Simulation", shader_path):
        sdk.Logger.error("Could not initialize the viewer application.")
        return

    app.set_solver(solver)
    app.set_mesh(mesh)

    sdk.Logger.info("Simulation started. Controls:")
    print("   - Right Mouse: Rotate Camera")
    print("   - Scroll: Zoom In/Out")
    print("   - ESC: Exit")

    app.run()
    app.shutdown()
    sdk.Logger.info("Simulation session closed.")

if __name__ == "__main__":
    run_curtain_simulation()