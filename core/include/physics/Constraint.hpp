#pragma once

#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

class Constraint {
public:
    virtual ~Constraint() = default;
    virtual void solve(std::vector<Particle>& particles) = 0;
};

}