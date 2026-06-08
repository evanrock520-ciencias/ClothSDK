// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/CapsuleCollider.hpp"

#include <Eigen/Dense>

#include "physics/Particle.hpp"

namespace Tissu {

CapsuleCollider::CapsuleCollider(double radius, const Eigen::Vector3d& start,
                                 const Eigen::Vector3d& end, double friction)
    : m_radius(radius), m_start(start), m_end(end) {
  m_friction = friction;
}

void CapsuleCollider::resolve(std::vector<Particle>& particles, double dt,
                              double thickness) {
  double collisionRadius = m_radius + thickness;
  double collisionRadiusSq = collisionRadius * collisionRadius;

  Eigen::Vector3d segment = m_end - m_start;
  double segmentLenSq = segment.squaredNorm();

  for (auto& particle : particles) {
    Eigen::Vector3d pos = particle.getPosition();
    Eigen::Vector3d pToA = pos - m_start;

    double t = 0.0;

    if (segmentLenSq > 1e-6) t = pToA.dot(segment) / segmentLenSq;

    if (t < 0.0) {
      t = 0.0;
    } else if (t > 1.0) {
      t = 1.0;
    }

    Eigen::Vector3d closestPoint = m_start + (segment * t);

    Eigen::Vector3d diff = pos - closestPoint;
    double distSq = diff.squaredNorm();

    if (distSq < collisionRadiusSq && distSq > 1e-9) {
      double dist = std::sqrt(distSq);
      Eigen::Vector3d normal = diff / dist;

      Eigen::Vector3d targetPos = closestPoint + (normal * collisionRadius);
      particle.setPosition(targetPos);

      Eigen::Vector3d colliderVelocity = getVelocityAtPoint(targetPos, dt);
      Eigen::Vector3d colliderDisplacement = colliderVelocity * dt;
      Eigen::Vector3d particleDisplacement = particle.getPosition() - particle.getOldPosition();
      Eigen::Vector3d relDisplacement = particleDisplacement - colliderDisplacement;

      double normalVelMag = relDisplacement.dot(normal);
      Eigen::Vector3d normalVel = normal * normalVelMag;
      Eigen::Vector3d tangentVel = relDisplacement - normalVel;

      Eigen::Vector3d newVelocity = normalVel + tangentVel * (1.0 - m_friction); 
      particle.setOldPosition(particle.getPosition() - newVelocity);
    }
  }
}

void CapsuleCollider::transform(const Eigen::Vector3d& position, const Eigen::Quaterniond& rotation) {
  m_start = position + rotation * (m_start - m_position);
  m_end = position + rotation * (m_end - m_position);
  Collider::transform(position, rotation);
}

}  // namespace Tissu