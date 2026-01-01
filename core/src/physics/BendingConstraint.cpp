#include "physics/BendingConstraint.hpp"
#include <algorithm>
#include <cmath>

namespace ClothSDK {

BendingConstraint::BendingConstraint(int idA, int idB, int idC, int idD, double restAngle, double compliance)
: m_idA(idA), m_idB(idB), m_idC(idC), m_idD(idD), m_restAngle(restAngle), m_compliance(compliance) {}

void BendingConstraint::solve(std::vector<Particle>& particles, double dt) {
    Particle& pA = particles[m_idA];
    Particle& pB = particles[m_idB];
    Particle& pC = particles[m_idC];
    Particle& pD = particles[m_idD];

    Eigen::Vector3d edgeVector = pB.getPosition() - pA.getPosition();
    double length = edgeVector.norm();
    Eigen::Vector3d CVector = pC.getPosition() - pA.getPosition();
    Eigen::Vector3d DVector = pD.getPosition() - pA.getPosition();

    Eigen::Vector3d normal1 = edgeVector.cross(CVector);
    Eigen::Vector3d normal2 = edgeVector.cross(DVector);

    double area1 = normal1.norm();
    double area2 = normal2.norm();

    if (area1 < 1e-6 || area2 < 1e-6 || length < 1e-6) return;

    double cosTheta = normal1.dot(normal2) / (area1 * area2);
    double clampedCos = std::clamp(cosTheta, -1.0, 1.0);
    double currentAngle = std::acos(clampedCos);

    if (normal1.squaredNorm() < 1e-6)
        return;

    Eigen::Vector3d gradC = (length / area1) * (normal1 / area1);
    Eigen::Vector3d gradD = (length / area2) * (normal2 / area2);

    double weightBC = (- (pB.getPosition() - pC.getPosition()).dot(edgeVector)) / length;
    double weightBD = (- (pB.getPosition() - pD.getPosition()).dot(edgeVector)) / length;
    double weightAC = (- (pA.getPosition() - pC.getPosition()).dot(edgeVector)) / length;
    double weightAD = (- (pA.getPosition() - pD.getPosition()).dot(edgeVector)) / length;

    Eigen::Vector3d gradA = -weightBC * gradC - weightBD * gradD;
    Eigen::Vector3d gradB = weightAC * gradC + weightAD * gradD;

    double wA = pA.getInverseMass();
    double wB = pB.getInverseMass();
    double wC = pC.getInverseMass();
    double wD = pD.getInverseMass();

    double wSum = wA * gradA.squaredNorm() + wB * gradB.squaredNorm() + wC * gradC.squaredNorm() + wD * gradD.squaredNorm();
    
    double alphaHat = m_compliance / (dt * dt);
    double deltaLambda = (-(currentAngle - m_restAngle) - alphaHat * m_lambda) / (wSum + alphaHat);
    m_lambda += deltaLambda;

    pA.setPosition(pA.getPosition() + wA * gradA * deltaLambda);
    pB.setPosition(pB.getPosition() + wB * gradB * deltaLambda);
    pC.setPosition(pC.getPosition() + wC * gradC * deltaLambda);
    pD.setPosition(pD.getPosition() + wD * gradD * deltaLambda);

}

}