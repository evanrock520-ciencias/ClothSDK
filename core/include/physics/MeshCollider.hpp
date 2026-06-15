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

 private:
  std::string m_meshPath;
  std::vector<Eigen::Vector3d> m_localVertices;
  std::vector<Eigen::Vector3d> m_worldVertices;
  std::vector<Triangle> m_triangles;
  BVH m_bvh;
};

}  // namespace Tissu