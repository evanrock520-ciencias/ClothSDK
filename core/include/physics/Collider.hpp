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
#include "Eigen/Dense"
#include "Eigen/src/Core/Matrix.h"

namespace Tissu {

class Particle;

/**
 * @class Collider
 * @brief Base interface for all geometric collision objects.
 *
 * Colliders in this engine follow the Extended Position Based Dynamics (XPBD)
 * approach. Instead of calculating complex contact forces, they project
 * penetrating particles back to the surface of the object and modify their
 * implicit velocity via friction.
 */
class Collider {
 public:
  /**
   * @brief Destroy the collider for safe cleanup.
   *
   */
  virtual ~Collider() = default;

  /**
   * @brief Detects and resolves interpenetration between particles and the
   * collider volume.
   *
   * Derived classes must implement the specific geometry projection logic.
   *
   * @param particles Reference to the global particle buffer.
   * @param dt Current substep time delta. Required for kinematic friction
   * calculations.
   */
  virtual void resolve(std::vector<Particle>& particles, double dt,
                       double thickness) = 0;

  /**
   * @brief Transforms the collider's position and rotation.
   *
   * @param position New position of the collider.
   * @param rotation New rotation of the collider.
   */
  virtual void transform(const Eigen::Vector3d& position, const Eigen::Quaterniond& rotation);

  Eigen::Vector3d getLinearVelocity(double dt) const;

  Eigen::Vector3d getAngularVelocity(double dt) const;

  Eigen::Vector3d getVelocityAtPoint(const Eigen::Vector3d& point, double dt) const;

  /**
   * @brief Configures the surface friction coefficient.
   *
   * @param friction Friction value in the range [0.0, 1.0]
   */
  void setFriction(double friction) { m_friction = friction; }

  inline void setPosition(const Eigen::Vector3d& position) { m_position = position; }
  inline void setRotation(const Eigen::Quaterniond& rotation) { m_rotation = rotation; }

  inline void setPrevPosition(const Eigen::Vector3d& prevPosition) { m_prevPosition = prevPosition; }
  inline void setPrevRotation(const Eigen::Quaterniond& prevRotation) { m_prevRotation = prevRotation; }

  /** @return The current surface friction coefficient. */
  inline double getFriction() const { return m_friction; }

  inline const Eigen::Vector3d& getPosition() const { return m_position; }
  inline const Eigen::Quaterniond& getRotation() const { return m_rotation; }

  inline const Eigen::Vector3d& getPrevPosition() const { return m_prevPosition; }
  inline const Eigen::Quaterniond& getPrevRotation() const { return m_prevRotation; }

 protected:
  /**
   * @brief Tangential friction coefficient used during collision response.
   *
   */
  double m_friction = 0.5;

  Eigen::Vector3d m_position = Eigen::Vector3d::Zero();
  Eigen::Quaterniond m_rotation = Eigen::Quaterniond::Identity();
  Eigen::Vector3d m_prevPosition = Eigen::Vector3d::Zero();
  Eigen::Quaterniond m_prevRotation = Eigen::Quaterniond::Identity();
};

}  // namespace Tissu