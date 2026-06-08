/*
 * Copyright 2026 Evan M.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <vector>

namespace Tissu {

class Particle;

/**
 * @class Force
 * @brief Base interface for all external forces in the simulation.
 *
 * Forces are applied once per substep, before position prediction.
 * They accumulate acceleration into each particle via @ref Particle::addForce,
 * which is then consumed by the Verlet integrator during @ref
 * Particle::integrate.
 *
 * Derived classes must implement @ref apply with their specific force model.
 * Current implementations include @ref GravityForce and @ref AerodynamicForce.
 */
class Force {
 public:
  /**
   * @brief Destroy the Force object.
   *
   */
  virtual ~Force() = default;

  /**
   * @brief Applies the force to all relevant particles in the buffer.
   *
   * Implementations should iterate over the particle buffer and call
   * @ref Particle::addForce on each affected particle. Stationary particles
   * (inverse mass == 0.0) should generally be skipped.
   *
   * This method is called by @ref Solver::applyForces once per substep,
   * before constraint solving begins.
   *
   * @param particles Reference to the solver's global particle buffer.
   * @param dt Current substep time delta in seconds.
   */
  virtual void apply(std::vector<Particle>& particles, double dt) = 0;
};

}  // namespace Tissu