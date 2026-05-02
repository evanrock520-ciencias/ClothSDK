# Physics And Math

This section describes the physical and mathematical foundations of Tissu.

## The Particle

A particle in Tissu is the simplest possible representation of matter: a point of mass in 3D space.
It represents just a position and a mass. Every piece of cloth is a collection of these points, connected by constraints that define how can move relative to each other.

![Particle](../videos/ParticleScene.gif)

## Motion

Computers work with discrete data, but physical phenomena are often
modeled with continuous mathematics. We cannot solve equations of motion
exactly, so we must approximate them numerically. Even though the
solutions are not exact, they are accurate enough to produce a
believable result.

There are 3 popular approaches to estimate a particle's position.

- Euler
- Verlet
- Ruggen-Kutta

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

### Verlet

The Verlet's method is given by the next one equation.

$$x_{n+1} = 2 ⋅ x_n - x_{n-1} + a_n ⋅ Δt²$$

Where:

- $x_n$ is the current position.
- $x_{n-1}$ is the previous position.
- $a_n$ is the current acceleration.
- $Δt$ is the timestep.
