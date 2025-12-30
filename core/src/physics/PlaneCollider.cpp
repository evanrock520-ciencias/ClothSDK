#include "physics/PlaneCollider.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

PlaneCollider::PlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction) 
: m_origin(origin), m_normal(normal.normalized()) {
    m_friction = friction;
}

void PlaneCollider::resolve(std::vector<Particle>& particles, double) {
    double thickness = 0.01;
    for(auto& particle : particles) {
        Eigen::Vector3d vec = particle.getPosition() - m_origin;
        double distance = vec.dot(m_normal);

        if (distance < thickness) {
            double penetration = thickness - distance;
            Eigen::Vector3d newPosition = particle.getPosition() + m_normal * penetration;
            particle.setPosition(newPosition);

            Eigen::Vector3d displacement = particle.getPosition() - particle.getOldPosition();
            Eigen::Vector3d normalDisp = m_normal * displacement.dot(m_normal);
            Eigen::Vector3d tangentialDisp = displacement - normalDisp;
            Eigen::Vector3d newDisplacement = normalDisp + tangentialDisp * (1.0 - m_friction);

            particle.setOldPosition(particle.getPosition() - newDisplacement);


        }
    }
}

}