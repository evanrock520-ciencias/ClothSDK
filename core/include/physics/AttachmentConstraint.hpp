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
#include <memory>
#include <vector>

#include "Eigen/Dense"
#include "physics/Collider.hpp"
#include "physics/Constraint.hpp"

namespace Tissu {

class MeshCollider;

class AttachmentConstraint : public Constraint {
public:
    AttachmentConstraint(int particleId, std::shared_ptr<Collider> collider,
                         int targetVertexId, double compliance = 0.0,
                         double restLength = 0.0);

    AttachmentConstraint(int particleId, std::shared_ptr<Collider> collider,
                         const Eigen::Vector3d& localAnchor,
                         double compliance = 0.0, double restLength = 0.0);

    void solve(std::vector<Particle>& particles, double dt) override;

    inline int getParticleId() const { return m_particleId; }
    std::vector<int> getParticleIds() const override { return {m_particleId}; }

private:
    Eigen::Vector3d computeWorldAnchor() const;

    int m_particleId;
    std::shared_ptr<Collider> m_collider;
    int m_targetVertexId = -1;
    Eigen::Vector3d m_localAnchor = Eigen::Vector3d::Zero();
    double m_restLength = 0.0;
};

} // namespace Tissu