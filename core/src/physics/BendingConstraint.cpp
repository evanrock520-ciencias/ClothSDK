#include "physics/BendingConstraint.hpp"
#include "utils/Logger.hpp"
#include <algorithm>
#include <cmath>

namespace ClothSDK {

BendingConstraint::BendingConstraint(int idA, int idB, int idC, int idD, double restAngle, double compliance)
: m_idA(idA), m_idB(idB), m_idC(idC), m_idD(idD), m_restAngle(restAngle) { m_compliance = compliance; }

void BendingConstraint::solve(std::vector<Particle>& particles, double dt) {
    if (dt < 1e-6) return;
    Particle& pA = particles[m_idA];
    Particle& pB = particles[m_idB];
    Particle& pC = particles[m_idC];
    Particle& pD = particles[m_idD];

    const Eigen::Vector3d& xA = pA.getPosition();
    const Eigen::Vector3d& xB = pB.getPosition();
    const Eigen::Vector3d& xC = pC.getPosition();
    const Eigen::Vector3d& xD = pD.getPosition();

    Eigen::Vector3d e = xB - xA;
    double len = e.norm();
    if (len < 1e-6) return;

    Eigen::Vector3d n1 = e.cross(xC - xA);
    Eigen::Vector3d n2 = e.cross(xD - xA);

    double n1_sq = n1.squaredNorm();
    double n2_sq = n2.squaredNorm();
    if (n1_sq < 1e-8 || n2_sq < 1e-8) return;

    double dot = n1.dot(n2) / std::sqrt(n1_sq * n2_sq);
    dot = std::clamp(dot, -1.0, 1.0);

    double angle = std::acos(dot);
    if (e.dot(n1.cross(n2)) < 0.0)
        angle = -angle;

    double C = angle - m_restAngle;

    Eigen::Vector3d gradC = (len / n1_sq) * n1;
    Eigen::Vector3d gradD = -(len / n2_sq) * n2;

    double s1 = (xC - xB).dot(e) / (len * len);
    double s2 = (xD - xB).dot(e) / (len * len);
    Eigen::Vector3d gradA = s1 * gradC + s2 * gradD;

    double t1 = (xA - xC).dot(e) / (len * len);
    double t2 = (xA - xD).dot(e) / (len * len);
    Eigen::Vector3d gradB = t1 * gradC + t2 * gradD;

    double wA = pA.getInverseMass();
    double wB = pB.getInverseMass();
    double wC = pC.getInverseMass();
    double wD = pD.getInverseMass();

    double alpha = m_compliance / (dt * dt);

    double denom =
        wA * gradA.squaredNorm() +
        wB * gradB.squaredNorm() +
        wC * gradC.squaredNorm() +
        wD * gradD.squaredNorm() +
        alpha;

    if (denom < 1e-8) return;

    double deltaLambda = -(C + alpha * m_lambda) / denom;
    m_lambda += deltaLambda;

    pA.setPosition(xA + wA * deltaLambda * gradA);
    pB.setPosition(xB + wB * deltaLambda * gradB);
    pC.setPosition(xC + wC * deltaLambda * gradC);
    pD.setPosition(xD + wD * deltaLambda * gradD);
}


}