#include "physics/Force.hpp"
#include "physics/Particle.hpp"

#pragma once

namespace ClothSDK {

class GravityForce : public Force {
public:
    explicit GravityForce(const Eigen::Vector3d& gravity)
        : m_gravity(gravity) {}
    
    void apply(std::vector<Particle>& particles, double dt) override;
private:
    Eigen::Vector3d m_gravity;
};

}