import pytest

from tissu import Material

def test_material_from_dic():
    properties = {
        "density" : 0.2,
        "bending_compliance" : 0.005,
        "shear_compliance" : 1e-8,
        "structural_compliance" : 1e-7
        }
    
    mat = Material.from_dict(properties)
    assert mat.density == 0.2
    assert mat.bending == 0.005
    assert mat.shear == 1e-8
    assert mat.structural == 1e-7

def test_material_from_preset():
    # Silk Preset:
    # "silk":    (0.1,  1e-9,  1e-8,  0.1)
    mat = Material.from_preset("silk")
    assert mat.density == 0.1
    assert mat.structural == 1e-9
    assert mat.shear == 1e-8
    assert mat.bending == 0.1
    
def test_material_from_non_existent_preset():
    with pytest.raises(ValueError):
        mat = Material.from_preset("linen")
    
def test_default_dic():
    mat = Material.from_dict({})
    
    assert mat.density == 0.1
    assert mat.structural == 1e-9
    assert mat.shear == 1e-8
    assert mat.bending == 0.01
    
def test_invalid_material_parameters():
    with pytest.raises(ValueError):
        properties = {
            "density" : 0
        }
        Material.from_dict(properties)
        
    with pytest.raises(ValueError):
        properties = {
            "density" : -0.2
        }
        Material.from_dict(properties)
    
    with pytest.raises(ValueError):
        properties = {
            "bending_compliance" : -0.01
        }
        Material.from_dict(properties)
    
    with pytest.raises(ValueError):
        properties = {
            "structural_compliance" : -1e-9
        }
        Material.from_dict(properties)
    
    with pytest.raises(ValueError):
        properties = {
            "shear_compliance" : -1e-8
        }
        Material.from_dict(properties)
        
def test_invalid_density_setter():
    mat = Material.from_dict({})
    with pytest.raises(ValueError):
        mat.density = -0.1
        