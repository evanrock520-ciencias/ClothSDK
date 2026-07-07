from tissu import engine

sim = engine.Simulation()
cotton_curtain = sim.create_grid(
    name="cotton_curtain", 
    rows=100,
    cols=100,
    spacing=0.05,
    material="silk",
    rotation=[0.7071, 0.0, 0.0, 0.7071],
    translation=[-2.5, 4.0, -2.5]
)

sim.add_floor("floor", -2.7)
sim.add_sphere("ball", center=[0.0, 0.0, 0.0], radius=1)

sim.view()