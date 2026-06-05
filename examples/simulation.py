from tissu import Simulation

def curtain():
    sim = Simulation(substeps=15, iterations=3, gravity=-9.81, thickness=0.05)
    sim.wind = [0.0, 7.0, 0.0]
    
    sim.add_floor(friction=0.5)
    curtain = sim.create_grid(
        name="curtain",
        rows=80,
        cols=80,
        spacing=0.05,
        material="silk"
    )
    
    curtain.pin_top_corners()
    sim.load_physics("data/configs/physics/realtime.json")
    print(curtain)
    
    @sim.on_frame(60)
    def unpin():
        curtain.unpin()
        
    sim.bake_alembic(
        filepath="data/animations/test.abc",
        start_frame=0,
        end_frame=120,
        fps=30
    )
    
def dress():
    sim = Simulation(substeps=40, iterations=2, gravity=-9.81, thickness=0.002)
    sim.wind = [0.0, 0.0, 0.0]
    
    sim.add_floor(height=-1.0, friction=1.0)

    pillow = sim.create_from_obj(
        name="dress",
        material="silk",
        path="data/models/dress.obj",
    )

    sim.save_scene("default.json", name="default")
    
def pillow():
    sim = Simulation(substeps=40, iterations=2, gravity=-9.81, thickness=0.002)
    sim.wind = [0.0, 0.0, 0.0]
    
    sim.add_floor(height=-1.0, friction=1.0)
    sim.add_sphere(name="sphere", center=[0.0, 0.0, 0.0], radius=0.8)

    pillow = sim.create_from_obj(
        name="pillow",
        material="cotton",
        path="data/models/pillow.obj",
    )
    
    pillow.update_material(
        structural=1e-7,  
        bending=1e-5     
    )
    
    rest_vol = pillow.enable_volume_preservation(compliance=0.0)
    
    sim.view()
    
def curtain_from_scene():
    sim = Simulation.load_scene("data/configs/scenes/.json")
    sim.view()
    
def save_scene():
    #DEBUG OPTION
    sim = Simulation(substeps=10, iterations=2, gravity=-9.81, thickness=0.1);
    
    sim.add_floor(friction=0.5)
    
    curtain = sim.create_grid(
        name="curtain",
        rows=80,
        cols=80,
        spacing=0.1,
        material="silk"
    )
    
    curtain.pin_top_corners()
        
    sim.save_scene("data/configs/scenes/save_test.json", "test")
    
if __name__ == "__main__":
    match 0:
        case 0 : curtain()
        case 1 : pillow()
        case 2 : curtain_from_scene()
        case 3: save_scene()
        case 4: dress()
        
