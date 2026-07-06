// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/AttachmentConstraint.hpp"

#include <utility>

#include "physics/MeshCollider.hpp"
#include "physics/Particle.hpp"

namespace Tissu {

AttachmentConstraint::AttachmentConstraint(int particleId,
                                           std::shared_ptr<Collider> collider,
                                           int targetVertexId,
                                           double compliance,
                                           double restLength)
    : m_particleId(particleId),
      m_collider(std::move(collider)),
      m_targetVertexId(targetVertexId),
      m_restLength(restLength) {
  m_compliance = compliance;
}

AttachmentConstraint::AttachmentConstraint(int particleId,
                                           std::shared_ptr<Collider> collider,
                                           const Eigen::Vector3d& localAnchor,
                                           double compliance,
                                           double restLength)
    : m_particleId(particleId),
      m_collider(std::move(collider)),
      m_localAnchor(localAnchor),
      m_restLength(restLength) {
  m_compliance = compliance;
}

Eigen::Vector3d AttachmentConstraint::computeWorldAnchor() const {
  // Vertex mode: read the world-space vertex directly from the MeshCollider
  if (m_targetVertexId >= 0) {
    auto meshCollider =
        std::dynamic_pointer_cast<MeshCollider>(m_collider);
    if (meshCollider) {
      const auto& worldVerts = meshCollider->getWorldVertices();
      if (m_targetVertexId < static_cast<int>(worldVerts.size())) {
        return worldVerts[m_targetVertexId];
      }
    }
  }

  // Local anchor mode: transform from collider-local to world space
  return m_collider->getPosition() + m_collider->getRotation() * m_localAnchor;
}

void AttachmentConstraint::solve(std::vector<Particle>& particles, double dt) {
  Particle& p = particles[m_particleId];

  Eigen::Vector3d worldAnchor = computeWorldAnchor();

  Eigen::Vector3d dir = p.getPosition() - worldAnchor;
  double dist = dir.norm();

  if (dist < 1e-6) return;

  Eigen::Vector3d n = dir / dist;
  double C = dist - m_restLength;

  double alphaHat = m_compliance / (dt * dt);
  double invMass = p.getInverseMass();
  double denominator = invMass + alphaHat;

  if (denominator < 1e-12) return;

  double deltaLambda = (-C - alphaHat * m_lambda) / denominator;
  m_lambda += deltaLambda;

  p.setPosition(p.getPosition() + n * (invMass * deltaLambda));
}

}  // namespace Tissu