#include "physics/SphereCollider.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

SphereCollider::SphereCollider(const Eigen::Vector3d& center, double radius, double friction)
    : m_center(center), m_radius(radius) 
{
    m_friction = friction;
}

void SphereCollider::resolve(std::vector<Particle>& particles, double dt, double thickness) {
    
    double collisionRadius = m_radius + thickness; 

    for (auto& particle : particles) {
        Eigen::Vector3d vec = particle.getPosition() - m_center;
        double distance = vec.norm();

        if (distance < 1e-6) {
            vec = Eigen::Vector3d::UnitY() * collisionRadius;
            distance = vec.norm();
        }

        if (distance < collisionRadius) {
            Eigen::Vector3d normal = vec.normalized();
            
            Eigen::Vector3d newPosition = m_center + normal * collisionRadius;
            particle.setPosition(newPosition);

            Eigen::Vector3d velocity = particle.getPosition() - particle.getOldPosition();
            
            double normalVelMag = velocity.dot(normal);
            Eigen::Vector3d normalVel = normal * normalVelMag;
            Eigen::Vector3d tangentVel = velocity - normalVel;

            Eigen::Vector3d newVelocity = normalVel + tangentVel * (1.0 - m_friction);

            particle.setOldPosition(particle.getPosition() - newVelocity);
        }
    }
}

}