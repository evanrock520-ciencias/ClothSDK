// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/ContactConstraint.hpp"
#include "physics/Particle.hpp"
#include <Eigen/Dense>

namespace ClothSDK {

ContactConstraint::ContactConstraint(int idA, int idB, double thickness, double compliance)
: m_idA(idA), m_idB(idB), m_thickness(thickness) { m_compliance = compliance; }

void ContactConstraint::solve(std::vector<Particle>& particles, double dt)
{
    Particle& pA = particles[m_idA];
    Particle& pB = particles[m_idB];

    Eigen::Vector3d d = pA.getPosition() - pB.getPosition();
    double dist = d.norm();

    if (dist >= m_thickness || dist < 1e-8)
        return;

    Eigen::Vector3d n = d / dist;

    double C = dist - m_thickness; 

    double wA = pA.getInverseMass();
    double wB = pB.getInverseMass();
    double wSum = wA + wB;
    if (wSum == 0.0)
        return;

    double correction = -C / wSum;

    pA.setPosition(pA.getPosition() + wA * correction * n);
    pB.setPosition(pB.getPosition() - wB * correction * n);
}


}