// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/SphereCollider.hpp"

#include "physics/Particle.hpp"

namespace Tissu {

SphereCollider::SphereCollider(const Eigen::Vector3d& center, double radius,
                               double friction)
    : m_center(center), m_radius(radius) {
    m_friction = friction;
    setPosition(center);
    setPrevPosition(center);
}

void SphereCollider::resolve(std::vector<Particle>& particles, double dt,
                             double thickness) {
    double collisionRadius = m_radius + thickness;
    Eigen::Vector3d linearVel = getLinearVelocity(dt);
    Eigen::Vector3d omega = getAngularVelocity(dt);

    for (auto& particle : particles) {
        Eigen::Vector3d vec = particle.getPosition() - m_center;
        double distance = vec.norm();

        if (distance <= 1e-6) {
            vec = Eigen::Vector3d::UnitY() * collisionRadius;
            distance = vec.norm();
        }

        if (distance <= collisionRadius) {
            Eigen::Vector3d normal = vec.normalized();

            Eigen::Vector3d newPosition = m_center + normal * collisionRadius;
            particle.setPosition(newPosition);

            Eigen::Vector3d colliderVelocity =
                linearVel + omega.cross(newPosition - m_position);
            Eigen::Vector3d colliderDisplacement = colliderVelocity * dt;
            Eigen::Vector3d particleDisplacement =
                particle.getPosition() - particle.getOldPosition();
            Eigen::Vector3d relDisplacement =
                particleDisplacement - colliderDisplacement;

            double normalVelMag = relDisplacement.dot(normal);
            Eigen::Vector3d normalVel = normal * normalVelMag;
            Eigen::Vector3d tangentVel = relDisplacement - normalVel;

            Eigen::Vector3d newVelocity =
                normalVel + tangentVel * (1.0 - m_friction);

            particle.setOldPosition(particle.getPosition() - newVelocity);
        }
    }
}

void SphereCollider::transform(const Eigen::Vector3d& position,
                               const Eigen::Quaterniond& rotation) {
    Collider::transform(position, rotation);
    m_center = position;
}

} // namespace Tissu