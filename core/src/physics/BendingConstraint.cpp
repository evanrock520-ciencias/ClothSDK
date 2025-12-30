#include "physics/BendingConstraint.hpp"
#include <algorithm>
#include <cmath>

namespace ClothSDK {

BendingConstraint::BendingConstraint(int idA, int idB, int idC, int idD, double restAngle, double stiffness)
: m_idA(idA), m_idB(idB), m_idC(idC), m_idD(idD), m_restAngle(restAngle), m_stiffness(stiffness) {}

void BendingConstraint::solve(std::vector<Particle>& particles) {
    Particle& pA = particles[m_idA];
    Particle& pB = particles[m_idB];
    Particle& pC = particles[m_idC];
    Particle& pD = particles[m_idD];

    Eigen::Vector3d edgeVector = pB.getPosition() - pA.getPosition();
    Eigen::Vector3d CVector = pC.getPosition() - pA.getPosition();
    Eigen::Vector3d DVector = pD.getPosition() - pA.getPosition();

    Eigen::Vector3d normalC = edgeVector.cross(CVector).normalized();
    Eigen::Vector3d normalD = edgeVector.cross(DVector).normalized();

    double cosTheta = normalC.dot(normalD);
    double clampedCos = std::clamp(cosTheta, -1.0, 1.0);
    double currentAngle = std::acos(clampedCos);

    double angleError = currentAngle - m_restAngle;
    double lambda = angleError * m_stiffness;

    pC.setPosition(pC.getPosition() - normalC * lambda * pC.getInverseMass());
    pD.setPosition(pD.getPosition() - normalD * lambda * pD.getInverseMass());
}

}