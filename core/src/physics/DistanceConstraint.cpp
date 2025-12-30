#include "physics/DistanceConstraint.hpp"

namespace ClothSDK {

DistanceConstraint::DistanceConstraint(int idA, int idB, double restLength, double stiffness)
: m_idA(idA), m_idB(idB), m_restLength(restLength), m_stiffness(stiffness) {}

void DistanceConstraint::solve(std::vector<Particle>& particles) {
    Particle& particle1 = particles[m_idA];
    Particle& particle2 = particles[m_idB];

    Eigen::Vector3d delta = particle1.getPosition() - particle2.getPosition();
    double currentLength = delta.norm();

    if (currentLength <= m_restLength) return;
    if (currentLength < 1e-6) return;

    double diff = (currentLength - m_restLength) / currentLength;
    double wA = particle1.getInverseMass();
    double wB = particle2.getInverseMass();
    double wSum = wA + wB;

    if(wSum == 0) return; 

    Eigen::Vector3d correctionVector = delta * diff * m_stiffness;

    Eigen::Vector3d deltaA = (wA / wSum) * correctionVector;
    Eigen::Vector3d deltaB = (wB / wSum) * correctionVector;

    particle1.setPosition(particle1.getPosition() - deltaA);
    particle2.setPosition(particle2.getPosition() + deltaB);
}

}
