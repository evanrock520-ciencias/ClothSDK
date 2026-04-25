import pytest

from tissu import Simulation, Material

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
    