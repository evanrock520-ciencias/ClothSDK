#include <Eigen/Dense>

#include "physics/CapsuleCollider.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

CapsuleCollider::CapsuleCollider(double radius, const Eigen::Vector3d& start, const Eigen::Vector3d& end, double friction)
    : m_radius(radius), m_start(start), m_end(end) {m_friction = friction; }

void CapsuleCollider::resolve(std::vector<Particle>& particles, double dt, double thickness) {
    double collisionRadius = m_radius + thickness;
    double collisionRadiusSq = collisionRadius * collisionRadius; 

    Eigen::Vector3d segment = m_end - m_start;
    double segmentLenSq = segment.squaredNorm();

    for (auto& particle : particles) {
        Eigen::Vector3d pos = particle.getPosition();
        Eigen::Vector3d pToA = pos - m_start;
        
        double t = 0.0;
        
        if (segmentLenSq > 1e-6) 
            t = pToA.dot(segment) / segmentLenSq;

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

        }
    }
}

}