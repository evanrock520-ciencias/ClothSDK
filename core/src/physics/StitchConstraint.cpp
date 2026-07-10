#include "physics/StitchConstraint.hpp"

namespace Tissu {

StitchConstraint::StitchConstraint(int idA, int idB, double compliance)
    : m_idA(idA), m_idB(idB) {
    m_compliance = compliance;
}

void StitchConstraint::solve(std::vector<Particle>& particles, double dt) {
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

    Eigen::Vector3d norm = delta / currentLength;
    double C = currentLength;
    double alphaHat = m_compliance / (dt * dt);
    double deltaLambda = (-C - alphaHat * m_lambda) / (wSum + alphaHat);
    m_lambda += deltaLambda;

    pA.setPosition(pA.getPosition() + wA * norm * deltaLambda);
    pB.setPosition(pB.getPosition() - wB * norm * deltaLambda);
}

} // namespace Tissu