#pragma once

#include "Particle.hpp"  
#include "Constraint.hpp"
#include "Collider.hpp"
#include "SpatialHash.hpp"
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
    void setWind(const Eigen::Vector3d& wind) {m_wind = wind; }
    void setAirDensity(double density) {m_airDensity = density; }

    void addDistanceConstraint(int idA, int idB, double compliance);
    void addBendingConstraint(int a, int b, int c, int d, double restAngle, double compliance);
    void addMassToParticle(int id, double mass);
    void addPlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction);
    void addSphereCollider(const Eigen::Vector3d& center, double radius, double friction);

    void addAeroFace(int idA, int idB, int idC);

    void update(double deltaTime);

private:
    void step(double dt);
    void applyForces(double dt);
    void predictPositions(double dt);
    void solveConstraints(double dt); 
    void applyAerodynamics(double dt);
    void solveSelfCollisions(double dt);

    struct AeroFace {
        int a, b, c;
    };

    std::vector<Particle> m_particles; 
    std::vector<std::unique_ptr<Constraint>> m_constraints;
    std::vector<std::unique_ptr<Collider>> m_colliders;
    SpatialHash m_spatialHash;
    Eigen::Vector3d m_gravity;
    int m_substeps;
    int m_iterations;
    std::vector<AeroFace> m_aeroFaces;
    Eigen::Vector3d m_wind;
    double m_airDensity;
    double m_time; 
    double m_thickness;
};

} 