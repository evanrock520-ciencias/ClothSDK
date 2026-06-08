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
#include "physics/Constraint.hpp"

namespace Tissu {

/**
 * @class PinConstraint
 * @brief Implementation of a positional pin constraint using XPBD.
 *
 * Anchors a single particle to a fixed point in world space, preventing it
 * from moving away from the target position. The constraint function is:
 * @f[
 * C(\mathbf{p}) = |\mathbf{p} - \mathbf{p}_{pin}|
 * @f]
 * A compliance of 0.0 produces a perfectly rigid pin. Higher values
 * allow the particle to drift, which can be used to simulate
 * elastic attachments or soft anchors.
 */
class PinConstraint : public Constraint {
 public:
  /**
   * @brief Constructs a pin constraint for a single particle.
   *
   * @param particleId Index of the target particle in the solver's particle
   * buffer.
   * @param pinPosition Fixed world-space anchor point the particle is
   * constrained to.
   * @param compliance Physical compliance @f$ \alpha @f$ in m/N.
   *                   Use 0.0 for a rigid pin, or a small positive value for a
   * soft anchor.
   */
  PinConstraint(int particleId, const Eigen::Vector3d& pinPosition,
                double compliance);

  /**
   * @brief Resolves the pin constraint by projecting the particle toward the
   * anchor.
   *
   * Computes the XPBD position correction using the distance from the particle
   * to the pin position as the constraint function. The Lagrange multiplier
   * increment is:
   * @f[
   * \Delta \lambda = \frac{-C(\mathbf{p}) - \tilde{\alpha}\lambda}{w +
   * \tilde{\alpha}}
   * @f]
   * where @f$ \tilde{\alpha} = \frac{\alpha}{\Delta t^2} @f$.
   *
   * @param particles Reference to the solver's global particle buffer.
   * @param dt Current substep time delta.
   */
  void solve(std::vector<Particle>& particles, double dt) override;

  /**
   * @brief Relocates the anchor point at runtime.
   *
   * Useful for animating pinned vertices, such as moving the corners
   * of a curtain along a rail or dragging a cloth interactively.
   *
   * @param newPos The new world-space anchor position.
   */
  inline void setPinPosition(const Eigen::Vector3d& newPos) {
    m_pinPos = newPos;
  }

  inline int getParticleId() const { return m_particleId; }

  std::vector<int> getParticleIds() const override { return {m_particleId}; }

 private:
  int m_particleId;  ///< Index of the constrained particle in the solver
                     ///< buffer.
  Eigen::Vector3d m_pinPos;  ///< Fixed world-space anchor position.
};

}  // namespace Tissu