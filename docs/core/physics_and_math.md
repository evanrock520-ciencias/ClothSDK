# Physics And Math

This section describes the physical and mathematical foundations used in ClothSDK.
The goal is not to be overly theoretical, but to explain the models and numerical methods clearly enough to understand the implementation and its limitations.

## Particle-Based Representation

This simulation is based on a particle system. Each particle represents a point mass with the following properties:

- Position
- Velocity
- Accumulated forces
- Mass

Particles interact through constraints, which together approximate the behavior of a continuous cloth surface.

## Newton's Second Law

The motion of each particle is governed by Newton's second law: $$F = ma$$

Which gives acceleration: $$a = \frac{F}{m}$$


## Verlet Integration

Verlet Integration is a numerical method used to integrate Newton's equations of motion. We consider the previous and the actual position to approximate the particle's velocity. 


## XPBD 

### Distance Constraint

### Dihedral Bending Constraint

## Colliders


