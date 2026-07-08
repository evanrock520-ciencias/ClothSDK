// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/Solver.hpp"

#include <omp.h>

#include <Eigen/Dense>
#include <algorithm>
#include <memory>

#include "engine/World.hpp"
#include "physics/AttachmentConstraint.hpp"
#include "physics/BendingConstraint.hpp"
#include "physics/Collider.hpp"
#include "physics/ContactConstraint.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/Force.hpp"
#include "physics/PinConstraint.hpp"
#include "physics/StitchConstraint.hpp"
#include "physics/VolumeConstraint.hpp"

namespace Tissu {
Solver::Solver()
    : m_substeps(15),
      m_iterations(2),
      m_collisionCompliance(1e-9),
      m_staticFriction(0.3),
      m_dynamicFriction(0.3),
      m_spatialHash(10007, 0.08),
      m_graphBuilt(false) {}

void Solver::update(World& world, double deltaTime) {
    if (m_particles.empty())
        return;

    if (!m_graphBuilt && !m_constraints.empty()) {
        buildGraph(42);
        m_graphBuilt = true;
    }

    m_spatialHash.setCellSize(world.getThickness());
    m_spatialHash.build(m_particles);

    const double substepDt = deltaTime / static_cast<double>(m_substeps);

    for (int i = 0; i < m_substeps; i++)
        step(world, substepDt);

    m_currentFrame++;
    m_currentTime += deltaTime;
}

void Solver::step(World& world, double dt) {
    applyForces(world, dt);

    predictPositions(dt);

    for (auto& constraint : m_constraints) {
        constraint->resetLambda();
    }

    for (int i = 0; i < m_iterations; i++) {
        solveConstraints(dt);
    }

    const auto& colliders = world.getColliders();
    for (auto& collider : colliders)
        collider->resolve(m_particles, dt, world.getThickness());

    solveSelfCollisions(dt, world.getThickness());
}

void Solver::predictPositions(double dt) {
    const int size = static_cast<int>(m_particles.size());
#pragma omp parallel for
    for (int i = 0; i < size; ++i) {
        m_particles[i].integrate(dt);
    }
}

int Solver::addParticle(const Particle& particle) {
    m_particles.push_back(particle);
    m_initialPositions.push_back(particle.getPosition());
    int id = static_cast<int>(m_particles.size() - 1);
    m_adjList.push_back({});
    return static_cast<int>(m_particles.size() - 1);
}

void Solver::softReset() {
    for (int i = 0; i < static_cast<int>(m_particles.size()); i++) {
        m_particles[i].setPosition(m_initialPositions[i]);
        m_particles[i].setOldPosition(m_initialPositions[i]);
    }

    m_currentFrame = 0;
    m_currentTime = 0.0;
}

void Solver::clear() {
    m_particles.clear();
    m_constraints.clear();
    m_adjList.clear();
    m_initialPositions.clear();
    m_batches.clear();
    m_transientPins.clear();
    m_attachments.clear();
    m_graphBuilt = false;
    m_currentFrame = 0;
    m_currentTime = 0.0;
}

const std::vector<Particle>& Solver::getParticles() const {
    return m_particles;
}

void Solver::addDistanceConstraint(int idA, int idB, double compliance) {
    const Particle& pA = m_particles[idA];
    const Particle& pB = m_particles[idB];
    double restLength = (pA.getPosition() - pB.getPosition()).norm();
    m_constraints.push_back(
        std::make_unique<DistanceConstraint>(idA, idB, restLength, compliance));
    addAdjacency(idA, idB);
    m_graphBuilt = false;
}

void Solver::addBendingConstraint(int idA, int idB, int idC, int idD,
                                  double restAngle, double compliance) {
    m_constraints.push_back(std::make_unique<BendingConstraint>(
        idA, idB, idC, idD, restAngle, compliance));
    addAdjacency(idA, idB);
    addAdjacency(idB, idC);
    addAdjacency(idA, idD);
    addAdjacency(idB, idD);
    m_graphBuilt = false;
}

void Solver::addStitch(int idA, int idB, double compliance) {
    m_constraints.push_back(
        std::make_unique<StitchConstraint>(idA, idB, compliance));
    addAdjacency(idA, idB);
    m_graphBuilt = false;
}

void Solver::addAttach(int id, std::shared_ptr<Collider> collider,
                       const Eigen::Vector3d& localAnchor, double compliance,
                       double restLength) {
    m_attachments.push_back(std::make_unique<AttachmentConstraint>(
        id, collider, localAnchor, compliance, restLength));
}

void Solver::addPin(int id, const Eigen::Vector3d& pos, double compliance) {
    m_transientPins.push_back(
        std::make_unique<PinConstraint>(id, pos, compliance));
}

void Solver::removePin(int id) {
    m_transientPins.erase(
        std::remove_if(m_transientPins.begin(), m_transientPins.end(),
                       [id](const std::unique_ptr<PinConstraint>& pin) {
                           return pin->getParticleId() == id;
                       }),
        m_transientPins.end());
}

void Solver::removeAttach(int id) {
    m_attachments.erase(
        std::remove_if(
            m_attachments.begin(), m_attachments.end(),
            [id](const std::unique_ptr<AttachmentConstraint>& attachment) {
                return attachment->getParticleId() == id;
            }),
        m_attachments.end());
}

void Solver::addAttachment(int particleId, std::shared_ptr<Collider> collider,
                           int targetVertexId, double compliance,
                           double restLength) {
    m_attachments.push_back(std::make_unique<AttachmentConstraint>(
        particleId, std::move(collider), targetVertexId, compliance,
        restLength));
}

void Solver::addAttachmentLocal(int particleId,
                                std::shared_ptr<Collider> collider,
                                const Eigen::Vector3d& localAnchor,
                                double compliance, double restLength) {
    m_attachments.push_back(std::make_unique<AttachmentConstraint>(
        particleId, std::move(collider), localAnchor, compliance, restLength));
}

void Solver::removeAttachment(int particleId) {
    m_attachments.erase(
        std::remove_if(
            m_attachments.begin(), m_attachments.end(),
            [particleId](const std::unique_ptr<AttachmentConstraint>& att) {
                return att->getParticleId() == particleId;
            }),
        m_attachments.end());
}

double Solver::addVolumeConstraint(const std::vector<Triangle>& triangles,
                                   const std::vector<Particle>& particles,
                                   double compliance) {
    auto constraint =
        std::make_unique<VolumeConstraint>(triangles, particles, compliance);
    const double restVolume = constraint->getRestVolume();
    m_constraints.push_back(std::move(constraint));
    m_graphBuilt = false;
    return restVolume;
}

void Solver::addMassToParticle(int id, double mass) {
    Particle& pA = m_particles[id];
    pA.addMass(mass);
}

void Solver::solveConstraints(double dt) {
    if (m_batches.empty()) {
        for (const auto& constraint : m_constraints)
            constraint->solve(m_particles, dt);
    } else {
        for (const auto& batch : m_batches) {
            const int batchSize = static_cast<int>(batch.size());
#pragma omp parallel for
            for (int i = 0; i < batchSize; ++i) {
                const int idx = batch[i];
                m_constraints[idx]->solve(m_particles, dt);
            }
        }
    }

    for (const auto& pin : m_transientPins) {
        pin->solve(m_particles, dt);
    }
    for (const auto& attach : m_attachments) {
        attach->solve(m_particles, dt);
    }
}

void Solver::solveSelfCollisions(double dt, double thickness) {
    for (int i = 0; i < static_cast<int>(m_particles.size()); ++i) {
        Particle& pA = m_particles[i];
        double wA = pA.getInverseMass();
        if (wA == 0.0)
            continue;

        m_spatialHash.query(m_particles, pA.getPosition(), thickness,
                            m_neighborsBuffer);

        for (int j : m_neighborsBuffer) {
            if (i >= j)
                continue;

            if (const auto& neighbors = m_adjList[i];
                std::binary_search(neighbors.begin(), neighbors.end(), j))
                continue;

            ContactConstraint contact(i, j, thickness, m_collisionCompliance,
                                      m_staticFriction, m_dynamicFriction);
            contact.solve(m_particles, dt);
        }
    }
}

void Solver::applyForces(World& world, double dt) {
    const auto& forces = world.getForces();
    for (auto& force : forces) {
        force->apply(m_particles, dt);
    }
}

void Solver::addAdjacency(int idA, int idB) {
    if (std::find(m_adjList[idA].begin(), m_adjList[idA].end(), idB) ==
        m_adjList[idA].end()) {
        m_adjList[idA].push_back(idB);
        std::sort(m_adjList[idA].begin(), m_adjList[idA].end());
    }

    if (std::find(m_adjList[idB].begin(), m_adjList[idB].end(), idA) ==
        m_adjList[idB].end()) {
        m_adjList[idB].push_back(idA);
        std::sort(m_adjList[idB].begin(), m_adjList[idB].end());
    }
}

void Solver::setIterations(const int count) {
    m_iterations = count;
}

void Solver::setSubsteps(const int count) {
    m_substeps = count;
}

void Solver::setParticleInverseMass(const int id, const double invMass) {
    m_particles[id].setInverseMass(invMass);
}

void Solver::buildGraph(const unsigned int seed) {
    m_graph.buildFrom(m_constraints, seed);
    m_batches = m_graph.colorBatches();
}

void Solver::invalidateGraph() {
    m_graphBuilt = false;
}
} // namespace Tissu
