#pragma once

#include <array>
#include <vector>

#include "data-structures/BVH.hpp"
#include "math/Types.hpp"
#include "physics/Collider.hpp"

namespace Tissu {

class MeshCollider : public Collider {
public:
    MeshCollider(const std::string& meshPath, double friction);

    MeshCollider(const std::vector<Eigen::Vector3d>& vertices,
                 const std::vector<std::array<int, 3>>& triangles,
                 double friction);

    void resolve(std::vector<Particle>& particles, double dt,
                 double thickness) override;

    void transform(const Eigen::Vector3d& position,
                   const Eigen::Quaterniond& rotation) override;

    const std::string& getMeshPath() const { return m_meshPath; }

    /** @brief World-space transformed vertices (updated by transform()). */
    const std::vector<Eigen::Vector3d>& getWorldVertices() const {
        return m_worldVertices;
    }
    /** @brief Triangle index list matching getWorldVertices(). */
    const std::vector<Triangle>& getTriangles() const { return m_triangles; }

private:
    std::string m_meshPath;
    std::vector<Eigen::Vector3d> m_localVertices;
    std::vector<Eigen::Vector3d> m_worldVertices;
    std::vector<Triangle> m_triangles;
    BVH m_bvh;
};

} // namespace Tissu