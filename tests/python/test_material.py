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