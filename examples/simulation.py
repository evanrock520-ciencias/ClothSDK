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
    sdk.Logger.info("   ClothSDK | Master Python Pipeline   ")
    sdk.Logger.info("========================================")

    solver = sdk.Solver()
    
    solver.set_gravity([0.0, -9.81, 0.0])
    solver.set_substeps(15)   
    solver.set_iterations(2)  
    
    solver.set_thickness(0.04)             
    solver.set_collision_compliance(1e-4)  
    
    solver.set_wind([1.0, 0.0, 3.0])
    solver.set_air_density(0.1)

    mesh = sdk.ClothMesh()
    
    mesh.set_material(0.1, 1e-9, 1e-7, 0.05)
    
    rows, cols = 100, 100
    spacing = 0.1
    
    sdk.Logger.info(f"Weaving {rows}x{cols} curtain grid...")
    mesh.init_grid(rows, cols, spacing, solver)

    top_row = rows - 1
    p_id = mesh.get_particle_id(top_row, 0)
    p_nid = mesh.get_particle_id(top_row, 99)
    
    solver.set_particle_inverse_mass(p_id, 0.0) 
    solver.set_particle_inverse_mass(p_nid, 0.0) 
    
    
    app = sdk.Application()
    
    app.set_solver(solver)
    app.set_mesh(mesh)

    if not app.init(1280, 720, "ClothSDK | Live XPBD Simulation", shader_path):
        sdk.Logger.error("Failed to initialize OpenGL context.")
        return

    sdk.Logger.info("Syncing topology with GPU...")
    app.sync_visual_topology()

    sdk.Logger.info("Simulating! Use mouse to orbit and ESC to close.")
    print("   - Right Click + Move: Orbit")
    print("   - Scroll: Zoom")
    print("   - Space: Pause/Resume")
    print("   - R: Reset Simulation")

    app.run()
    
    app.shutdown()
    sdk.Logger.info("Simulation session finished.")

if __name__ == "__main__":
    run_curtain_simulation()
