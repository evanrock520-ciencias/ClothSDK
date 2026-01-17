#pragma once

#include "physics/Constraint.hpp"
namespace ClothSDK {

class ContactConstraint : public Constraint {
public:
    ContactConstraint(int idA, int idB, double thickness, double compliance);
    void solve(std::vector<Particle>& particles, double dt) override;
private:
    int m_idA;
    int m_idB;
    double m_thickness;
};

}