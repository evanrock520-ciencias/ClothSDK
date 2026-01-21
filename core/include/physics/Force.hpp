#pragma once
#include <vector>

namespace ClothSDK {

class Particle;

class Force {
public:
    virtual ~Force() = default;

    virtual void apply(std::vector<Particle>& particles, double dt) = 0;
};

}