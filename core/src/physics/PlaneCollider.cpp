// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/PlaneCollider.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

PlaneCollider::PlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction) 
: m_origin(origin), m_normal(normal.normalized()) {
    m_friction = friction;
}

void PlaneCollider::resolve(std::vector<Particle>& particles, double dt, double thickness) {
    
    for(auto& particle : particles) {
        Eigen::Vector3d vec = particle.getPosition() - m_origin;
        double distance = vec.dot(m_normal);

        if (distance < thickness) {
            
            double penetration = thickness - distance;
            Eigen::Vector3d newPosition = particle.getPosition() + m_normal * penetration;
            particle.setPosition(newPosition);

            Eigen::Vector3d velocity = particle.getPosition() - particle.getOldPosition();
            
            double normalVelMag = velocity.dot(m_normal);
            Eigen::Vector3d normalVel = m_normal * normalVelMag;
            Eigen::Vector3d tangentVel = velocity - normalVel;

            Eigen::Vector3d newVelocity = normalVel + tangentVel * (1.0 - m_friction);

            particle.setOldPosition(particle.getPosition() - newVelocity);
        }
    }
}

}