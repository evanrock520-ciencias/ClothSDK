import os
import sys
import time
from load_path import load

load()
try:
    import cloth_sdk as sdk
    import numpy as np
except ImportError as e:
    print(f"Error importing modules: {e}")
    sys.exit(1)

def run_alembic_simulation():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    output_dir = os.path.join(project_root, "data", "animations")
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        
    output_filename = os.path.join(output_dir, "curtain_wind_sim.abc")

    sdk.Logger.info("========================================")
    sdk.Logger.info("   ClothSDK | Alembic Export Pipeline   ")
    sdk.Logger.info("========================================")

    solver = sdk.Solver()
    solver.set_gravity([0.0, -9.81, 0.0])
    solver.set_substeps(15)   
    solver.set_iterations(2)  
    
    solver.set_thickness(0.04)             
    solver.set_collision_compliance(1e-4)  
    solver.set_wind([2.0, 0.0, 6.0])
    solver.set_air_density(0.15)

    mesh = sdk.ClothMesh()
    mesh.set_material(0.1, 1e-10, 1e-9, 0.01)
    
    rows, cols = 120, 90 
    spacing = 0.1
    
    sdk.Logger.info(f"Weaving {rows}x{cols} cloth grid (10,800 particles)...")
    mesh.init_grid(rows, cols, spacing, solver)

    particles = solver.get_particles()
    top_row = rows - 1

    p_id_left = mesh.get_particle_id(top_row, 0)
    pos_left = particles[p_id_left].get_position()
    solver.add_pin(p_id_left, pos_left, 0.0) 
    
    p_id_right = mesh.get_particle_id(top_row, cols - 1)
    pos_right = particles[p_id_right].get_position()
    solver.add_pin(p_id_right, pos_right, 0.0)

    sdk.Logger.info(f"Pinned corners using PinConstraints.")

    exporter = sdk.AlembicExporter()
    
    initial_pos = [p.get_position() for p in particles]
    indices = mesh.get_triangles()

    sdk.Logger.info(f"Creating Alembic Archive: {output_filename}")
    if not exporter.open(output_filename, initial_pos, indices):
        sdk.Logger.error("Failed to open Alembic file for writing!")
        return

    total_frames = 240
    fps = 60.0
    dt = 1.0 / fps
    
    start_wall_time = time.time()
    sdk.Logger.info(f"Exporting {total_frames} frames...")

    for f in range(total_frames):
        solver.update(dt)
        
        current_pos = [p.get_position() for p in solver.get_particles()]
        
        exporter.write_frame(current_pos, f * dt)
        
        if f % 20 == 0:
            sdk.Logger.info(f"      Frame {f}/{total_frames} written...")

    exporter.close()
    
    end_wall_time = time.time()
    sdk.Logger.info("========================================")
    sdk.Logger.info(f"   EXPORT COMPLETE: {output_filename}")
    sdk.Logger.info(f"   Total Time: {end_wall_time - start_wall_time:.2f} seconds")
    sdk.Logger.info("========================================")

if __name__ == "__main__":
    run_alembic_simulation()