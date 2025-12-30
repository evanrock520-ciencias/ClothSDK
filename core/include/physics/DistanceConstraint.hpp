#pragma once

#include "Constraint.hpp"
#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

class DistanceConstraint : public Constraint {
public:
    DistanceConstraint(int idA, int idB, double restLength, double compliance);
    void solve(std::vector<Particle>& particles, double dt) override;

private:
    int m_idA, m_idB;
    double m_restLength;
    double m_compliance;
};

}