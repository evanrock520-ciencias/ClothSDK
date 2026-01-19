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
    sdk.Logger.info("========================================")
    sdk.Logger.info("   ClothSDK | Alembic Export Pipeline   ")
    sdk.Logger.info("========================================")

    solver = sdk.Solver()
    
    solver.set_gravity([0.0, -9.81, 0.0])
    solver.set_substeps(15)   
    solver.set_iterations(2)  
    
    solver.set_thickness(0.04)             
    solver.set_collision_compliance(1e-4)  
    solver.set_wind([2.0, 0.0, 8.0])
    solver.set_air_density(0.15)

    mesh = sdk.ClothMesh()
    
    mesh.set_material(0.15, 1e-10, 1e-8, 0.005)
    
    rows, cols = 50, 50 
    spacing = 0.1
    
    sdk.Logger.info(f"Weaving {rows}x{cols} cloth grid...")
    mesh.init_grid(rows, cols, spacing, solver)

    top_row = rows - 1
    for c in range(cols):
        p_id = mesh.get_particle_id(top_row, c)
        solver.set_particle_inverse_mass(p_id, 0.0)
    
    sdk.Logger.info(f"Pinned {cols} vertices at the top rail.")

    exporter = sdk.AlembicExporter()
    output_filename = "curtain_wind_sim.abc"
    
    particles = solver.get_particles()
    initial_pos = [p.get_position() for p in particles]
    
    indices = mesh.get_triangles()

    sdk.Logger.info(f"Creating Alembic Archive: {output_filename}")
    if not exporter.open(output_filename, initial_pos, indices):
        sdk.Logger.error("Failed to open Alembic file for writing!")
        return

    total_frames = 120 
    fps = 60.0
    dt = 1.0 / fps
    
    sdk.Logger.info(f"Starting Export of {total_frames} frames...")
    start_wall_time = time.time()

    for f in range(total_frames):
        solver.update(dt)
        
        current_particles = solver.get_particles()
        current_pos = [p.get_position() for p in current_particles]
        
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