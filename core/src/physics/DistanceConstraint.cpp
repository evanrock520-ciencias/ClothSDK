#include "physics/DistanceConstraint.hpp"

namespace ClothSDK {

DistanceConstraint::DistanceConstraint(int idA, int idB, double restLength, double compliance)
: m_idA(idA), m_idB(idB), m_restLength(restLength), m_compliance(compliance) {}

void DistanceConstraint::solve(std::vector<Particle>& particles, double dt) {
    Particle& particle1 = particles[m_idA];
    Particle& particle2 = particles[m_idB];

    Eigen::Vector3d delta = particle1.getPosition() - particle2.getPosition();
    double currentLength = delta.norm();

    if (currentLength <= m_restLength) return;
    if (currentLength < 1e-6) return;

    double wA = particle1.getInverseMass();
    double wB = particle2.getInverseMass();
    double wSum = wA + wB;

    if(wSum == 0) return; 

    Eigen::Vector3d deltaNormalized = delta / currentLength;
    double diff = currentLength - m_restLength;
    double alphaHat = m_compliance / (dt * dt);
    double deltaLambda = (-diff - alphaHat * m_lambda) / (wSum + alphaHat);
    m_lambda += deltaLambda;

    particle1.setPosition(particle1.getPosition() + wA * deltaNormalized * deltaLambda);
    particle2.setPosition(particle2.getPosition() - wB * deltaNormalized * deltaLambda);

}

}
