#pragma once

#include "Constraint.hpp"

#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

class BendingConstraint : public Constraint {
public:
    BendingConstraint(int idA, int idB, int idc, int idD, double restAngle, double compliance);

    void solve(std::vector<Particle>& particles, double dt) override;

private:
    int m_idA, m_idB, m_idC, m_idD;
    double m_restAngle;
};

}