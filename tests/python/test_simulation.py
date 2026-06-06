import pytest

from tissu import Simulation, Material
import tempfile

def test_initial_parameters():
    sim = Simulation()
    
    assert sim.gravity == -9.81
    assert sim.substeps >= 1
    assert sim.iterations >= 1
    
def test_correct_invalid_parameters():
    sim = Simulation(substeps=0, thickness=-0.1)
    
    assert sim.substeps == 1
    assert sim.thickness == 0.001
    
def test_invalid_shape_wind_vector():
    sim = Simulation()
    with pytest.raises(ValueError):
        sim.wind = [0.0, 0.0]
    with pytest.raises(ValueError):
        sim.wind = [0.0, 0.0, 0.0, 0.0]
        
def test_repeated_fabric_name():
    sim = Simulation()
    
    sim.create_grid(
        name="tissu",
        rows=10,
        cols=10,
        spacing=0.01,
        material="silk"
    )
    
    with pytest.raises(ValueError):
        sim.create_grid(
            name="tissu",
            rows=20,
            cols=20,
            spacing=0.1,
            material="cotton"
        )
        
def test_reset():
    sim = Simulation()
    for i in range(10):
        sim.create_grid(
            name=f"curtain_{i}",
            rows=50,
            cols=50,
            spacing=0.01,
            material="silk"
        )
    
    sim.reset()
    
    assert len(sim.cloth_objects) == 0
    assert len(sim._aero_forces) == 0
    
def test_bake_without_fabrics():
    sim = Simulation()
    
    with pytest.raises(RuntimeError):
        sim.bake_alembic(
            filepath="test.abc",
        )

def test_snapshot_of_invalid_fabric():
    sim = Simulation()
    
    with pytest.raises(RuntimeError):
        sim.save_snapshot(
            filename="test.obj",
            fabric_name="curtain"
        )
        
def test_resolve_material():
    # Default Material
    cotton = Simulation._resolve_material(None)
    assert isinstance(cotton, Material)
    
    silk = Simulation._resolve_material("silk")
    assert isinstance(silk, Material)
    
    default = Simulation._resolve_material({})
    assert isinstance(default, Material)

    with pytest.raises(TypeError):
        Simulation._resolve_material(123)
    
def test_pin():
    sim = Simulation(substeps=15, iterations=3, gravity=-9.81, thickness=0.05)
    sim.wind = [0.0, 7.0, 0.0]
    rows = 80
    
    sim.add_floor(friction=0.5)
    curtain = sim.create_grid(
        name="curtain",
        rows=rows,
        cols=40,
        spacing=0.05,
        material="silk"
    )
    
    curtain.pin_top_corners()
    pins = curtain.get_pins()
    assert len(pins) == 2
    
    curtain.pin_by_height()
    pins = curtain.get_pins() # The upper edge
    assert len(pins) == rows 
    
    curtain.unpin()
    pins = curtain.get_pins()
    assert len(pins) == 0

def test_on_frame():
    sim = Simulation(substeps=15, iterations=3, gravity=-9.81, thickness=0.05)
    sim.add_floor(friction=0.5)
    curtain = sim.create_grid(
        name="curtain",
        rows=20,
        cols=20,
        spacing=0.05,
        material="silk"
    )
    
    curtain.pin_top_corners()
    pins = curtain.get_pins()
    assert len(pins) == 2
    frames = 60
    event_frame = 40
    
    @sim.on_frame(event_frame)
    def unpin():
        curtain.unpin()
    
    sim.simulate(41)
    pins = curtain.get_pins()
    assert len(pins) == 0
    