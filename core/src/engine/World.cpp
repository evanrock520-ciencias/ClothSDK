// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "engine/World.hpp"

#include <memory>

#include "physics/CapsuleCollider.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"

namespace Tissu {

World::World()
    : m_airDensity(0.1), m_gravity(0.0, -9.81, 0.0), m_thickness(0.02) {}

void World::addCloth(std::shared_ptr<Cloth> cloth) {
  m_cloths.push_back(cloth);
}

void World::addCollider(std::shared_ptr<Collider> collider) {
  m_colliders.push_back(collider);
}

void World::addForce(std::shared_ptr<Force> force) {
  m_forces.push_back(force);
}

void World::clear() {
  m_cloths.clear();
  m_colliders.clear();
  m_forces.clear();
}

void World::addPlaneCollider(const Eigen::Vector3d& origin,
                             const Eigen::Vector3d& normal, double friction) {
  m_colliders.push_back(
      std::make_unique<PlaneCollider>(origin, normal, friction));
}

void World::addSphereCollider(const Eigen::Vector3d& center, double radius,
                              double friction) {
  m_colliders.push_back(
      std::make_unique<SphereCollider>(center, radius, friction));
}

void World::addCapsuleCollider(const Eigen::Vector3d start,
                               const Eigen::Vector3d end, double radius,
                               double friction) {
  m_colliders.push_back(
      std::make_unique<CapsuleCollider>(radius, start, end, friction));
}

void World::moveCollider(size_t index, const Eigen::Vector3d& newPosition, const Eigen::Quaterniond& newRotation) {
  if (index < m_colliders.size()) 
    m_colliders[index]->transform(newPosition, newRotation);
}

}  // namespace Tissu