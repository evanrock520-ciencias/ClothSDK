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

#include <Eigen/Dense>
#include <memory>
#include <unordered_set>
#include <vector>

#include "AttachmentConstraint.hpp"
#include "Constraint.hpp"
#include "Particle.hpp"
#include "PinConstraint.hpp"
#include "data-structures/SpatialHash.hpp"
#include "math/Types.hpp"
#include "physics/ConstraintGraph.hpp"

namespace Tissu {

class World;

class Solver {
public:
    Solver();

    int addParticle(const Particle& p);
    void clear();
    const std::vector<Particle>& getParticles() const;
    std::vector<Particle>& getParticles() { return m_particles; }
    void setParticleInverseMass(int id, double invMass);
    void addMassToParticle(int id, double mass);

    void setSubsteps(int count);
    void setIterations(int count);
    void setCollisionCompliance(double c) { m_collisionCompliance = c; }
    void setCurrentFrame(int frame) { m_currentFrame = frame; }
    void setCurrentTime(double time) { m_currentTime = time; }

    inline int getSubsteps() const { return m_substeps; }
    inline int getIterations() const { return m_iterations; }
    inline double getCollisionCompliance() const {
        return m_collisionCompliance;
    }
    inline int getParticleCount() const {
        return static_cast<int>(m_particles.size());
    }
    inline int getCurrentFrame() const { return m_currentFrame; }
    inline double getCurrentTime() const { return m_currentTime; }
    inline const std::vector<std::unique_ptr<Constraint>>&
    getConstraints() const {
        return m_constraints;
    }

    void addDistanceConstraint(int idA, int idB, double compliance);
    void addBendingConstraint(int a, int b, int c, int d, double restAngle,
                              double compliance);
    double addVolumeConstraint(const std::vector<Triangle>& triangles,
                               const std::vector<Particle>& particles,
                               double compliance);
    void addPin(int id, const Eigen::Vector3d& pos, double compliance = 0.0);
    void addStitch(int idA, int idB, double compliance);
    void addAttach(int id, std::shared_ptr<Collider> collider,
                   const Eigen::Vector3d& localAnchor, double compliance,
                   double restLength);
    void removeAttach(int id);
    void removePin(int id);
    void addAttachment(int particleId, std::shared_ptr<Collider> collider,
                       int targetVertexId, double compliance = 0.0,
                       double restLength = 0.0);
    void addAttachmentLocal(int particleId, std::shared_ptr<Collider> collider,
                            const Eigen::Vector3d& localAnchor,
                            double compliance = 0.0, double restLength = 0.0);
    void removeAttachment(int particleId);
    void buildGraph(unsigned int seed);
    void invalidateGraph();

    void softReset();

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
    std::vector<Eigen::Vector3d> m_initialPositions;

    SpatialHash m_spatialHash;
    std::vector<int> m_neighborsBuffer;

    int m_substeps;
    int m_iterations;
    double m_collisionCompliance;

    ConstraintGraph m_graph;
    std::vector<std::vector<int>> m_batches;
    bool m_graphBuilt;

    int m_currentFrame = 0;
    double m_currentTime = 0.0;

    std::vector<std::unique_ptr<PinConstraint>> m_transientPins;
    std::vector<std::unique_ptr<AttachmentConstraint>> m_attachments;
};

} // namespace Tissu