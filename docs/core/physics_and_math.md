# Physics And Math

This section describes the physical and mathematical foundations of Tissu.

## The Particle

A particle in Tissu is the simplest possible representation of matter: a point of mass in 3D space.
It represents just a position and a mass. Every piece of cloth is a collection of these points, connected by constraints that define how can move relative to each other.

![Particle](../videos/ParticleScene.gif)

## Motion

Computers work with discrete data, but physical phenomena are often
modeled with continuous mathematics. In particular Newton's laws of motion are modeled
this way.

$$F = m \ddot{x}$$

We cannot solve equations of motion exactly, so we must approximate them numerically.
Even though the solutions are not exact, they are accurate enough to produce a
believable result.

There are 3 popular approaches to estimate a particle's position.

- Euler
- Verlet
- Runge-Kutta

### Euler

The Euler's method is given by the next equations.

$$x_{n+1} = x_n + v_n ⋅ Δt$$
$$v_{n+1} = v_n + a_n ⋅ Δt$$

Where:

- $x_n$ is the current position.
- $v_n$ is the current velocity.
- $a_n$ is the current acceleration.
- $Δt$ is the timestep.

This is likely the most naïve approach to solve the problem.

Euler's method is an example of a stepwise method — $x_{n+1}$ and
$v_{n+1}$ at $t + \Delta t$ depend only on values at $t$. The local
truncation error grows as $O(\Delta t^2)$, which means the method is
stable when $\Delta t$ is small enough. However, as $\Delta t$ increases
the error accumulates and the simulation tends to overshoot, adding
energy to the system each step instead of conserving it.

In cloth simulation this is critical. A solver runs hundreds of substeps
per second, and an unstable integrator would cause particles to explode
almost immediately. This is why we need a more stable approach.

![Euler Example](../videos/Euler.gif)

### Verlet

The Verlet's method is given by the next one equation.

$$x_{n+1} = 2 ⋅ x_n - x_{n-1} + a_n ⋅ Δt²$$

Where:

- $x_n$ is the current position.
- $x_{n-1}$ is the previous position.
- $a_n$ is the current acceleration.
- $Δt$ is the timestep.

Verlet is a widely used integrator in physics simulation and computer graphics,
valued for its stability and low computational cost. Unlike Euler, Verlet is nearly
symplectic — meaning it approximately conserves the total energy of the system over time,
preventing the artificial energy drift that makes Euler unreliable for cloth simulation.

This is a **two-step** integrator: the next position is estimated from both the current and
the previous position, rather than relying solely on velocity and $Δt$.
As a consequence, velocity is implicit — it does not appear in the update equation directly.
When needed, it can be recovered as:
$$v_n = \frac{x_n - x_{n-1}}{Δt}$$

Verlet's equation can be derived from a Taylor Series expansion. This derivation reveals that
the local truncation error on position is $O(Δt⁴)$, which is significantly better than Euler's
$O(Δt²)$. The global error over a full simulation remains $O(Δt²)$, but the method's symmetry
suppresses the energy drift that accumulates with non-symplectic integrators.

![Verlet Example](../videos/Verlet.gif)
Tissu uses Verlet integration as it's core integration.

```cpp

void Particle::integrate(double deltaTime) {
    Eigen::Vector3d velocity = (m_position - m_oldPosition) * m_damping;
    Eigen::Vector3d currentPos = m_position;

    m_position = m_position + velocity + m_acceleration * (deltaTime * deltaTime);
    
    m_oldPosition = currentPos;
    clearForces();
}
```

However, integration alone is not enough. We still need a way to enforce physical rules
that define how cloth behaves.

<!---TODO: Explain PDB behavior -->

## PBD
