#include "physics/BendingConstraint.hpp"
#include "utils/Logger.hpp"
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

    Eigen::Vector3d e = pB.getPosition() - pA.getPosition();
    double len = e.norm();
    if (len < 1e-6) return;

    Eigen::Vector3d n1 = e.cross(pC.getPosition() - pA.getPosition());
    Eigen::Vector3d n2 = e.cross(pD.getPosition() - pA.getPosition());

    double n1_sq = n1.squaredNorm();
    double n2_sq = n2.squaredNorm();

    if (n1_sq < 1e-8 || n2_sq < 1e-8) return;

    double dot = n1.dot(n2) / std::sqrt(n1_sq * n2_sq);
    dot = std::clamp(dot, -1.0, 1.0);
    double angle = std::acos(dot);
    
    Eigen::Vector3d cross_n = n1.cross(n2);
    if (e.dot(cross_n) < 0) angle = -angle;

    Eigen::Vector3d gradC = (len / n1_sq) * n1;
    Eigen::Vector3d gradD = -(len / n2_sq) * n2; 

    double s1 = (pC.getPosition() - pB.getPosition()).dot(e) / (len * len); 
    double s2 = (pD.getPosition() - pB.getPosition()).dot(e) / (len * len);
    
    Eigen::Vector3d gradA = s1 * gradC + s2 * gradD;
    
    double t1 = (pA.getPosition() - pC.getPosition()).dot(e) / (len * len);
    double t2 = (pA.getPosition() - pD.getPosition()).dot(e) / (len * len);
    
    Eigen::Vector3d gradB = t1 * gradC + t2 * gradD;

}

}