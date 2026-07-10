#include "physics/VolumeConstraint.hpp"

#include "physics/Particle.hpp"

namespace Tissu {

VolumeConstraint::VolumeConstraint(const std::vector<Triangle>& triangles,
                                   const std::vector<Particle>& particles,
                                   double compliance)
    : m_triangles(triangles) {
    m_compliance = compliance;
    double sum = 0;
    for (auto& tri : triangles) {
        Eigen::Vector3d posA = particles[tri.a].getPosition();
        Eigen::Vector3d posB = particles[tri.b].getPosition();
        Eigen::Vector3d posC = particles[tri.c].getPosition();

        sum += posA.dot(posB.cross(posC));
    }

    m_restVolume = std::abs(sum / 6.0);
}

double
VolumeConstraint::computeVolume(const std::vector<Particle>& particles) const {
    double sum = 0;
    for (auto& tri : m_triangles) {
        Eigen::Vector3d posA = particles[tri.a].getPosition();
        Eigen::Vector3d posB = particles[tri.b].getPosition();
        Eigen::Vector3d posC = particles[tri.c].getPosition();

        sum += posA.dot(posB.cross(posC));
    }
    return std::abs(sum / 6.0);
}

void VolumeConstraint::solve(std::vector<Particle>& particles, double dt) {
    double currentVolume = computeVolume(particles);
    double C = (currentVolume / m_restVolume) - 1.0;

    if (std::abs(C) < 1e-6)
        return;

    std::vector<Eigen::Vector3d> gradients(particles.size(),
                                           Eigen::Vector3d::Zero());

    for (auto& tri : m_triangles) {
        Eigen::Vector3d posA = particles[tri.a].getPosition();
        Eigen::Vector3d posB = particles[tri.b].getPosition();
        Eigen::Vector3d posC = particles[tri.c].getPosition();

        gradients[tri.a] += posB.cross(posC) / 6.0;
        gradients[tri.b] += posC.cross(posA) / 6.0;
        gradients[tri.c] += posA.cross(posB) / 6.0;
    }

    double alphaHat = m_compliance / (dt * dt);
    double denom = alphaHat;
    for (size_t i = 0; i < particles.size(); i++) {
        denom += particles[i].getInverseMass() * gradients[i].squaredNorm();
    }

    double deltaLambda = (-C - alphaHat * m_lambda) / denom;
    m_lambda += deltaLambda;

    for (size_t i = 0; i < particles.size(); i++) {
        if (gradients[i].isZero())
            continue;

        double w = particles[i].getInverseMass();
        particles[i].setPosition(particles[i].getPosition() +
                                 w * deltaLambda * gradients[i]);
    }
}

} // namespace Tissu