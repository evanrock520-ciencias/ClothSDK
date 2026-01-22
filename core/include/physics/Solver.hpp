#pragma once

#include "Particle.hpp"  
#include "Constraint.hpp"
#include "SpatialHash.hpp"
#include "engine/World.hpp" 
#include <unordered_set>
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
    void setParticleInverseMass(int id, double invMass);
    void addMassToParticle(int id, double mass);

    void setSubsteps(int count);
    void setIterations(int count); 
    void setCollisionCompliance(double c) { m_collisionCompliance = c; }
    
    inline int getSubsteps() const { return m_substeps; }
    inline int getIterations() const { return m_iterations; }
    inline double getCollisionCompliance() const { return m_collisionCompliance; }
    inline int getParticleCount() const { return static_cast<int>(m_particles.size()); }

    void addDistanceConstraint(int idA, int idB, double compliance);
    void addBendingConstraint(int a, int b, int c, int d, double restAngle, double compliance);
    void addPin(int id, const Eigen::Vector3d& pos, double compliance = 0.0);

    void update(World& world, double deltaTime);

private:
    void step(World& world, double dt);
    void applyForces(World& world, double dt);
    void solveSelfCollisions(double dt, double thickness); 

    void predictPositions(double dt);
    void solveConstraints(double dt); 
    uint64_t getAdjacencyKey(int idA, int idB) const;

    std::vector<Particle> m_particles; 
    std::vector<std::unique_ptr<Constraint>> m_constraints;
    std::unordered_set<uint64_t> m_adjacencies;
    
    SpatialHash m_spatialHash;
    std::vector<int> m_neighborsBuffer;

    int m_substeps;
    int m_iterations;
    double m_collisionCompliance;
};

} 