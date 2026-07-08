// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/ContactConstraint.hpp"

#include <Eigen/Dense>

#include "physics/Particle.hpp"

namespace Tissu {

ContactConstraint::ContactConstraint(int idA, int idB, double thickness,
                                     double compliance, double staticFriction,
                                     double dynamicFriction)
    : m_idA(idA),
      m_idB(idB),
      m_thickness(thickness),
      m_staticFriction(staticFriction),
      m_dynamicFriction(dynamicFriction) {
    m_compliance = compliance;
}

void ContactConstraint::solve(std::vector<Particle>& particles, double dt) {
    Particle& pA = particles[m_idA];
    Particle& pB = particles[m_idB];

    const double wA = pA.getInverseMass();
    const double wB = pB.getInverseMass();
    const double wSum = wA + wB;

    const double alphaHat = m_compliance / (dt * dt);

    if (wSum + alphaHat < 1e-12)
        return;

    const Eigen::Vector3d d = pA.getPosition() - pB.getPosition();
    const double distSq = d.squaredNorm();

    if (const double thicknessSq = m_thickness * m_thickness;
        distSq >= thicknessSq || distSq < 1e-12)
        return;

    const double dist = std::sqrt(distSq);
    Eigen::Vector3d n = d / dist;

    const double C = dist - m_thickness;
    const double deltaLambda = -C / (wSum + alphaHat);
    const Eigen::Vector3d corr = n * deltaLambda;

    const Eigen::Vector3d deltaA = pA.getPosition() - pA.getOldPosition();
    const Eigen::Vector3d deltaB = pB.getPosition() - pB.getOldPosition();
    const Eigen::Vector3d deltaRel = deltaA - deltaB;
    const Eigen::Vector3d deltaTangent = deltaRel - deltaRel.dot(n) * n;

    if (const double lamdaT = deltaTangent.norm(); lamdaT > 1e-12) {
        if (lamdaT < m_staticFriction * deltaLambda) {
            pA.setPosition(pA.getPosition() - deltaTangent * (wA / wSum));
            pB.setPosition(pB.getPosition() + deltaTangent * (wB / wSum));
        } else {
            const Eigen::Vector3d vA =
                (pA.getPosition() - pA.getOldPosition()) / dt;
            const Eigen::Vector3d vB =
                (pB.getPosition() - pB.getOldPosition()) / dt;
            const Eigen::Vector3d vRel = vA - vB;
            const Eigen::Vector3d vt = vRel - vRel.dot(n) * n;

            if (const double vt_norm = vt.norm(); vt_norm > 1e-12) {
                const double dv_mag =
                    std::min(m_dynamicFriction * deltaLambda / dt, vt_norm);
                const Eigen::Vector3d dv = -(vt / vt_norm) * dv_mag;

                pA.setOldPosition(pA.getOldPosition() - dv * (wA / wSum) * dt);
                pB.setOldPosition(pB.getOldPosition() + dv * (wB / wSum) * dt);
            }
        }
    }

    pA.setPosition(pA.getPosition() + corr * wA);
    pB.setPosition(pB.getPosition() - corr * wB);
}

} // namespace Tissu