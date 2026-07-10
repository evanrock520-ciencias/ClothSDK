#include "physics/MeshCollider.hpp"

#include "io/OBJLoader.hpp"
#include "math/Geometry.hpp"
#include "physics/Particle.hpp"

namespace Tissu {

MeshCollider::MeshCollider(const std::string& meshPath, double friction)
    : m_meshPath(meshPath) {
    m_friction = friction;

    std::vector<Eigen::Vector3d> positions;
    std::vector<int> indices;

    if (!OBJLoader::load(meshPath, positions, indices))
        throw std::runtime_error("Could not load mesh: " + meshPath);

    m_localVertices = positions;
    m_worldVertices = positions;

    m_triangles.reserve(indices.size() / 3);
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
        m_triangles.emplace_back(indices[i], indices[i + 1], indices[i + 2]);

    m_bvh.build(m_worldVertices, m_triangles);
}

MeshCollider::MeshCollider(const std::vector<Eigen::Vector3d>& vertices,
                           const std::vector<std::array<int, 3>>& triangles,
                           double friction)
    : m_localVertices(vertices), m_worldVertices(vertices) {
    m_friction = friction;

    m_triangles.reserve(triangles.size());
    for (const auto& tri : triangles) {
        m_triangles.emplace_back(tri[0], tri[1], tri[2]);
    }

    m_bvh.build(m_worldVertices, m_triangles);
}

void MeshCollider::transform(const Eigen::Vector3d& position,
                             const Eigen::Quaterniond& rotation) {
    Collider::transform(position, rotation);

    for (size_t i = 0; i < m_localVertices.size(); ++i)
        m_worldVertices[i] = rotation * m_localVertices[i] + position;

    m_bvh.build(m_worldVertices, m_triangles);
}

void MeshCollider::resolve(std::vector<Particle>& particles, double dt,
                           double thickness) {
    Eigen::Vector3d linearVel = getLinearVelocity(dt);
    Eigen::Vector3d omega = getAngularVelocity(dt);

    for (auto& particle : particles) {
        int triIdx =
            m_bvh.closestTriangle(particle.getPosition(), m_worldVertices);
        if (triIdx == -1)
            continue;

        const Triangle& tri = m_bvh.getTriangle(triIdx);
        const Eigen::Vector3d& a = m_worldVertices[tri.a];
        const Eigen::Vector3d& b = m_worldVertices[tri.b];
        const Eigen::Vector3d& c = m_worldVertices[tri.c];

        Eigen::Vector3d cp =
            closestPointOnTriangle(particle.getPosition(), a, b, c);
        Eigen::Vector3d toParticle = particle.getPosition() - cp;
        double distance = toParticle.norm();

        if (distance <= thickness) {
            Eigen::Vector3d normal = (distance > 1e-6)
                                         ? toParticle.normalized()
                                         : ((b - a).cross(c - a)).normalized();

            Eigen::Vector3d newPosition = cp + normal * thickness;
            particle.setPosition(newPosition);

            Eigen::Vector3d colliderVelocity =
                linearVel + omega.cross(newPosition - m_position);
            Eigen::Vector3d colliderDisplacement = colliderVelocity * dt;
            Eigen::Vector3d particleDisplacement =
                particle.getPosition() - particle.getOldPosition();
            Eigen::Vector3d relDisplacement =
                particleDisplacement - colliderDisplacement;

            double normalVelMag = relDisplacement.dot(normal);
            Eigen::Vector3d normalVel = normal * normalVelMag;
            Eigen::Vector3d tangentVel = relDisplacement - normalVel;

            Eigen::Vector3d newVelocity =
                normalVel + tangentVel * (1.0 - m_friction);

            particle.setOldPosition(particle.getPosition() - newVelocity);
        }
    }
}

} // namespace Tissu
