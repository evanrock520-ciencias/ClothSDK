#pragma once

#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

class Constraint {
public:
    Constraint() : m_lambda(0.0), m_compliance(0.0) {}
    
    virtual ~Constraint() = default;

    virtual void solve(std::vector<Particle>& particles, double dt) = 0;

    virtual void resetLambda() { m_lambda = 0.0; }

protected:
    double m_lambda;      
    double m_compliance; 
};

}