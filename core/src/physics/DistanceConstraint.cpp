// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/DistanceConstraint.hpp"

namespace ClothSDK {

DistanceConstraint::DistanceConstraint(int idA, int idB, double restLength, double compliance)
: m_idA(idA), m_idB(idB), m_restLength(restLength), m_compliance(compliance) {}

void DistanceConstraint::solve(std::vector<Particle>& particles, double dt) {
    Particle& pA = particles[m_idA];
    Particle& pB = particles[m_idB];

    Eigen::Vector3d delta = pA.getPosition() - pB.getPosition();
    double currentLength = delta.norm();

    if (currentLength < 1e-6)
        return;

    double wA = pA.getInverseMass();
    double wB = pB.getInverseMass();
    double wSum = wA + wB;
    if (wSum == 0.0)
        return;

    Eigen::Vector3d n = delta / currentLength;
    double C = currentLength - m_restLength;  

    double alphaHat = m_compliance / (dt * dt);
    double denominator = wSum + alphaHat;
    if (denominator < 1e-12) return; 
    double deltaLambda = (-C - alphaHat * m_lambda) / denominator;
    m_lambda += deltaLambda;

    pA.setPosition(pA.getPosition() + wA * n * deltaLambda);
    pB.setPosition(pB.getPosition() - wB * n * deltaLambda);
}

}
