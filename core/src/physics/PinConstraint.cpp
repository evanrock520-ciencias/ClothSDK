// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <Eigen/Dense>

#include "physics/PinConstraint.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

PinConstraint::PinConstraint(int particleId, const Eigen::Vector3d& pinPosition, double compliance) : m_particleId(particleId), m_pinPos(pinPosition) { m_compliance = compliance; }

void PinConstraint::solve(std::vector<Particle>& particles, double dt) {
    Particle& p = particles[m_particleId];
    Eigen::Vector3d dir = p.getPosition() - m_pinPos;
    double dist = dir.norm();

    if (dist < 1e-6) return;

    Eigen::Vector3d n = dir / dist;

    double alphaHat = m_compliance / (dt * dt);
    double invMass = p.getInverseMass();
    double denominator = invMass + alphaHat;
    
    if (denominator < 1e-12) return; 

    double deltaLambda = (-dist - alphaHat * m_lambda) / denominator;
    m_lambda += deltaLambda;

    p.setPosition(p.getPosition() + n * (p.getInverseMass() * deltaLambda));
}

}