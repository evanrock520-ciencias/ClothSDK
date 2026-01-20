/*
 * Copyright 2026 Evan M.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Particle.hpp"  
#include "Constraint.hpp"
#include "Collider.hpp"
#include "SpatialHash.hpp"
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

    void setGravity(const Eigen::Vector3d& gravity);
    void setSubsteps(int count);
    void setIterations(int count); 
    void setParticleInverseMass(int id, double invMass);
    void setWind(const Eigen::Vector3d& wind) {m_wind = wind; }
    void setAirDensity(double density) {m_airDensity = density; }
    void setThickness(double thickness) { m_thickness = thickness; }
    void setCollisionCompliance(double c) { m_collisionCompliance = c; }

    void addDistanceConstraint(int idA, int idB, double compliance);
    void addBendingConstraint(int a, int b, int c, int d, double restAngle, double compliance);
    void addPin(int id, const Eigen::Vector3d& pos, double compliance = 0.0);
    void addMassToParticle(int id, double mass);
    void addPlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction);
    void addSphereCollider(const Eigen::Vector3d& center, double radius, double friction);
    void addAeroFace(int idA, int idB, int idC);

    void update(double deltaTime);

    int getSubsteps() const { return m_substeps; }
    int getIterations() const { return m_iterations; }
    const Eigen::Vector3d& getGravity() const { return m_gravity; }
    double getAirDensity() const { return m_airDensity; }
    const Eigen::Vector3d& getWind() const { return m_wind; }
    double getThickness() const { return m_thickness; }
    double getCollisionCompliance() const { return m_collisionCompliance; }

private:
    void step(double dt);
    void applyForces(double dt);
    void predictPositions(double dt);
    void solveConstraints(double dt); 
    void applyAerodynamics(double dt);
    void solveSelfCollisions(double dt);
    uint64_t getAdjacencyKey(int idA, int idB) const;

    struct AeroFace {
        int a, b, c;
    };

    std::vector<Particle> m_particles; 
    std::vector<std::unique_ptr<Constraint>> m_constraints;
    std::vector<std::unique_ptr<Collider>> m_colliders;
    std::vector<int> m_neighborsBuffer;
    std::unordered_set<uint64_t> m_adjacencies;
    SpatialHash m_spatialHash;
    Eigen::Vector3d m_gravity;
    int m_substeps;
    int m_iterations;
    std::vector<AeroFace> m_aeroFaces;
    Eigen::Vector3d m_wind;
    double m_airDensity;
    double m_time; 
    double m_thickness;
    double m_collisionCompliance;
};

} 