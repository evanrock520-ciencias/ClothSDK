// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/Collider.hpp"

namespace Tissu {
void Collider::transform(const Eigen::Vector3d& position,
                         const Eigen::Quaterniond& rotation) {
    m_prevPosition = m_position;
    m_prevRotation = m_rotation;

    m_position = position;
    m_rotation = rotation;
}

Eigen::Vector3d Collider::getLinearVelocity(double dt) const {
    if (dt <= 1e-6)
        return Eigen::Vector3d::Zero();
    return (m_position - m_prevPosition) / dt;
}

Eigen::Vector3d Collider::getAngularVelocity(double dt) const {
    if (dt <= 1e-6)
        return Eigen::Vector3d::Zero();

    Eigen::Quaterniond deltaRot = m_rotation * m_prevRotation.conjugate();
    Eigen::AngleAxisd angleAxis(deltaRot);

    return angleAxis.axis() * angleAxis.angle() / dt;
}

Eigen::Vector3d Collider::getVelocityAtPoint(const Eigen::Vector3d& point,
                                             double dt) const {
    Eigen::Vector3d linearVel = getLinearVelocity(dt);
    Eigen::Vector3d omega = getAngularVelocity(dt);
    return linearVel + omega.cross(point - m_position);
}
} // namespace Tissu