#pragma once

#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

class Solver {
public:
    Solver();

    void addParticle(const Particle& p);
    void clear();

    const std::vector<Particle>& getParticles() const;

    void setGravity(const Eigen::Vector3d gravity);
    void setSubsteps(int count);
    
    void update(double deltaTime);

private:
    void step(double dt);
    void applyForces();
    void predictPositions(double dt);

    std::vector<Particle> listParticles;
    Eigen::Vector3d m_gravity;
    int m_substeps;
};

}