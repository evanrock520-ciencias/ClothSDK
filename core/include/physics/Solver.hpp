#pragma once

#include "Particle.hpp"  
#include "Constraint.hpp"
#include "Collider.hpp"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace ClothSDK {

class Solver {
public:
    Solver();

    int addParticle(const Particle& p);
    void clear();

    const std::vector<Particle>& getParticles() const;

    void setGravity(const Eigen::Vector3d& gravity);
    void setSubsteps(int count);
    void setIterations(int count); 
    void setParticleInverseMass(int id, double invMass);

    void addDistanceConstraint(int idA, int idB, double stiffness);
    void addMassToParticle(int id, double mass);
    void addPlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction);

    void update(double deltaTime);

private:
    void step(double dt);
    void applyForces();
    void predictPositions(double dt);
    void solveConstraints(); 

    std::vector<Particle> m_particles; 
    std::vector<std::unique_ptr<Constraint>> m_constraints;
    std::vector<std::unique_ptr<Collider>> m_colliders;
    Eigen::Vector3d m_gravity;
    int m_substeps;
    int m_iterations; 
};

} 