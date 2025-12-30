#include "physics/SphereCollider.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

SphereCollider::SphereCollider(const Eigen::Vector3d& center, double radius, double friction)
    : m_center(center), m_radius(radius) 
{
    m_friction = friction;
}

void SphereCollider::resolve(std::vector<Particle>& particles, double dt) {
    double thickness = 0.01;
    for (auto& particle : particles) {
        Eigen::Vector3d vec = particle.getPosition() - m_center;
        double distance = vec.norm();

        if (distance < 1e-6) {
            vec = Eigen::Vector3d::UnitY() * (m_radius + thickness);
            distance = vec.norm();
        }


        if (distance < (m_radius + thickness)) {
            Eigen::Vector3d normal = vec.normalized();
            Eigen::Vector3d newPosition = m_center + normal * (m_radius + thickness);
            particle.setPosition(newPosition);

            Eigen::Vector3d displacement = particle.getPosition() - particle.getOldPosition();
            Eigen::Vector3d normalDisp = normal * displacement.dot(normal);
            Eigen::Vector3d tangentialDisp = displacement - normalDisp;
            Eigen::Vector3d newDisplacement = normalDisp + tangentialDisp * (1.0 - m_friction);

            particle.setOldPosition(particle.getPosition() - newDisplacement);

        }
    }
}

}