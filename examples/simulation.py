from load_path import load
load() 

import cloth_sdk as sdk
import os

solver = sdk.Solver()
print(solver.get_gravity())