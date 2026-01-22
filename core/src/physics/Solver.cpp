// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <omp.h>

#include "physics/Solver.hpp"
#include "engine/World.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/BendingConstraint.hpp"
#include "physics/Collider.hpp"
#include "physics/Force.hpp"
#include "physics/PinConstraint.hpp"
#include <Eigen/Dense>
#include <memory>

namespace ClothSDK {
    Solver::Solver()
    : m_substeps(15), m_iterations(2), m_collisionCompliance(1e-9), m_spatialHash(10007, 0.08) {}

    void Solver::update(World& world, double deltaTime) {
        if (m_particles.empty()) return;

        m_spatialHash.setCellSize(world.getThickness()); 
        m_spatialHash.build(m_particles);

        double substepDt = deltaTime / static_cast<double>(m_substeps);
        
        for (int i = 0; i < m_substeps; i++) {
            step(world, substepDt);
        }
    }

    void Solver::step(World& world, double dt) {
        applyForces(world, dt);

        predictPositions(dt);

        for (auto& constraint : m_constraints) {
            constraint->resetLambda();
        }

        for (int i = 0; i < m_iterations; i++) {
            for(auto& constraint : m_constraints) {
                constraint->solve(m_particles, dt);
            }
        }

        const auto& colliders = world.getColliders();
        for (auto& collider : colliders) {
            collider->resolve(m_particles, dt, world.getThickness());
        }

        solveSelfCollisions(dt, world.getThickness());
    }

    void Solver::predictPositions(double dt) {
        #pragma omp parallel for
        for (auto& particle : m_particles) {
            particle.integrate(dt);
        }
    }

    int Solver::addParticle(const Particle& particle) {
        m_particles.push_back(particle);
        return static_cast<int>(m_particles.size() - 1);
    }

    void Solver::clear() {
        m_particles.clear();
        m_constraints.clear();
        m_adjacencies.clear();
    }

    const std::vector<Particle>& Solver::getParticles() const {
        return m_particles;
    }

    void Solver::addDistanceConstraint(int idA, int idB, double compliance) {
        Particle& pA = m_particles[idA];
        Particle& pB = m_particles[idB];
        double restLength = (pA.getPosition() - pB.getPosition()).norm();
        m_constraints.push_back(std::make_unique<DistanceConstraint>(idA, idB, restLength, compliance));
        m_adjacencies.insert(getAdjacencyKey(idA, idB));
    }

    void Solver::addBendingConstraint(int idA, int idB, int idC, int idD, double restAngle, double compliance) {
        m_constraints.push_back(std::make_unique<BendingConstraint>(idA, idB, idC, idD, restAngle, compliance));
        m_adjacencies.insert(getAdjacencyKey(idA, idC));
        m_adjacencies.insert(getAdjacencyKey(idB, idC));
        m_adjacencies.insert(getAdjacencyKey(idA, idD));
        m_adjacencies.insert(getAdjacencyKey(idB, idD));
    }

    void Solver::addPin(int id, const Eigen::Vector3d& pos, double compliance) {
        m_constraints.push_back(std::make_unique<PinConstraint>(id, pos, compliance));
    }

    void Solver::addMassToParticle(int id, double mass) {
        Particle& pA = m_particles[id];
        pA.addMass(mass);
    }

    void Solver::solveConstraints(double dt) {
        for(auto& constraint : m_constraints)
            constraint->solve(m_particles, dt);
    }

    void Solver::solveSelfCollisions(double dt, double thickness) {
        double alphaHat = m_collisionCompliance / (dt * dt);
        double thicknessSq = thickness * thickness;

        for (int i = 0; i < (int)m_particles.size(); ++i) {
            Particle& pA = m_particles[i];
            double wA = pA.getInverseMass();
            if (wA == 0.0) continue;

            m_spatialHash.query(m_particles, pA.getPosition(), thickness, m_neighborsBuffer);

            for (int j : m_neighborsBuffer) {
                if (i >= j) continue; 

                if (m_adjacencies.count(getAdjacencyKey(i, j))) continue;

                Particle& pB = m_particles[j];
                double wB = pB.getInverseMass();
                double wSum = wA + wB;

                if (wSum + alphaHat < 1e-12) continue;

                Eigen::Vector3d dir = pA.getPosition() - pB.getPosition();
                double distSq = dir.squaredNorm();

                if (distSq > 0.0 && distSq < thicknessSq) {
                    double dist = std::sqrt(distSq);
                    Eigen::Vector3d normal = dir / dist;

                    double C = dist - thickness;
                    
                    double deltaLambda = -C / (wSum + alphaHat);
                    Eigen::Vector3d corr = normal * deltaLambda;

                    pA.setPosition(pA.getPosition() + corr * wA);
                    pB.setPosition(pB.getPosition() - corr * wB);
                }
            }
        }
    }

    void Solver::applyForces(World& world, double dt) {
        const auto& forces = world.getForces();
        for (auto& force : forces) {
            force->apply(m_particles, dt);
        }
    }

    uint64_t Solver::getAdjacencyKey(int idA, int idB) const{
        uint64_t low = static_cast<uint32_t>(std::min(idA, idB));
        uint64_t high = static_cast<uint32_t>(std::max(idA, idB));

        return (high << 32) | low;
    }

    void Solver::setIterations(int count) {
        m_iterations = count;
    }

    void Solver::setSubsteps(int count) {
        m_substeps = count;
    }

    void Solver::setParticleInverseMass(int id, double invMass) {
        m_particles[id].setInverseMass(invMass);
    }
}
