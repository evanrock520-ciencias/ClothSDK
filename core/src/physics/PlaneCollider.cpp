// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/PlaneCollider.hpp"

#include "physics/Particle.hpp"

namespace Tissu {

PlaneCollider::PlaneCollider(const Eigen::Vector3d& origin,
                             const Eigen::Vector3d& normal, double friction)
    : m_origin(origin), m_normal(normal.normalized()) {
  m_friction = friction;
  setPosition(origin);
  setPrevPosition(origin);
}

void PlaneCollider::resolve(std::vector<Particle>& particles, double dt,
                            double thickness) {
  Eigen::Vector3d linearVel = getLinearVelocity(dt);
  Eigen::Vector3d omega = getAngularVelocity(dt);

  for (auto& particle : particles) {
    Eigen::Vector3d vec = particle.getPosition() - m_origin;
    double distance = vec.dot(m_normal);

    if (distance <= thickness) {
      double penetration = thickness - distance;
      Eigen::Vector3d newPosition =
          particle.getPosition() + m_normal * penetration;
      particle.setPosition(newPosition);

      Eigen::Vector3d colliderVelocity = linearVel + omega.cross(newPosition - m_position);
      Eigen::Vector3d colliderDisplacement = colliderVelocity * dt;
      Eigen::Vector3d particleDisplacement = particle.getPosition() - particle.getOldPosition();
      Eigen::Vector3d relDisplacement = particleDisplacement - colliderDisplacement;

      double normalVelMag = relDisplacement.dot(m_normal);
      Eigen::Vector3d normalVel = m_normal * normalVelMag;
      Eigen::Vector3d tangentVel = relDisplacement - normalVel;

      Eigen::Vector3d newVelocity = normalVel + tangentVel * (1.0 - m_friction);

      particle.setOldPosition(particle.getPosition() - newVelocity);
    }
  }
}

void PlaneCollider::transform(const Eigen::Vector3d& position, const Eigen::Quaterniond& rotation) {
  m_origin = position;
  m_normal = rotation * m_normal;
  Collider::transform(position, rotation);
}

}  // namespace Tissu