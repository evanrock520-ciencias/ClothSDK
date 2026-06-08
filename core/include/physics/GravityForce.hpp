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

#include "physics/Force.hpp"
#include "physics/Particle.hpp"

namespace Tissu {

/**
 * @class GravityForce
 * @brief Applies a constant gravitational acceleration to all dynamic
 * particles.
 *
 * This is a uniform body force — every particle with a non-zero inverse mass
 * receives the same acceleration vector regardless of its position.
 * Pinned or static particles (inverse mass == 0.0) are automatically skipped.
 *
 * The standard Earth gravity vector is @f$ (0, -9.81, 0) @f$ m/s²,
 * but any direction can be used to simulate non-standard environments
 * such as zero-gravity or lateral pull.
 */
class GravityForce : public Force {
public:
  /**
   * @brief Constructs a GravityForce with a given acceleration vector.
   *
   * @param gravity World-space acceleration vector in m/s².
   *                Defaults to @f$ (0, -9.81, 0) @f$ in typical usage.
   *                Pass @f$ (0, 0, 0) @f$ to disable gravity at runtime.
   */
  explicit GravityForce(const Eigen::Vector3d &gravity) : m_gravity(gravity) {}

  /**
   * @brief Accumulates the gravitational acceleration into each dynamic
   * particle.
   *
   * Iterates over the full particle buffer and calls @ref Particle::addForce
   * with @ref m_gravity on every particle whose inverse mass is non-zero.
   *
   * @param particles Reference to the solver's global particle buffer.
   * @param dt Current substep time delta in seconds.
   */
  void apply(std::vector<Particle> &particles, double dt) override;

  /**
   * @brief Updates the gravitational acceleration vector at runtime.
   *
   * @param gravity New world-space acceleration vector in m/s².
   */
  inline void setGravity(const Eigen::Vector3d &gravity) {
    m_gravity = gravity;
  }

  /**
   * @brief Gets the current gravitational acceleration vector.
   *
   * @return The world-space acceleration vector in m/s².
   */
  inline const Eigen::Vector3d &getGravity() const { return m_gravity; }

private:
  Eigen::Vector3d
      m_gravity; //< Constant gravitational acceleration vector in m/s².
};

} // namespace Tissu