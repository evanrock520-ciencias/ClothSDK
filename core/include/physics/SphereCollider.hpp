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

#include "Collider.hpp"
#include <Eigen/Dense>

namespace ClothSDK {

/**
 * @class SphereCollider
 * @brief Implementation of a spherical collision volume.
 * 
 * This class handles particle-sphere intersection by projecting any penetrating 
 * particles along the radial vector originating from the sphere's center. 
 * It provides a dynamic collision normal that varies based on the particle's 
 * relative position.
 */
class SphereCollider : public Collider {
public:
    /**
     * @brief Constructs a new Sphere Collider.
     * 
     * @param center The world-space center point of the sphere.
     * @param radius The radius of the sphere in world units.
     * @param friction The friction coefficient.
     */
    SphereCollider(const Eigen::Vector3d& center, double radius, double friction);

    /**
     * @brief Resolves collisions between the sphere and a buffer of particles.
     * 
     * The algorithm follows these steps:
     * 1. Calculate the distance from the particle to the sphere center.
     * 2. If distance < (radius + thickness), project the particle to the surface.
     * 3. Calculate the local collision normal as the normalized radial vector.
     * 4. Apply tangential friction to the particle's implicit velocity.
     *
     * @param particles Reference to the global particle buffer.
     * @param dt Current substep time delta.
     */
    void resolve(std::vector<Particle>& particles, double dt, double thickness);

private:
    Eigen::Vector3d m_center;   ///< The center point of the sphere in 3D space.
    double m_radius;            ///< Radius of the collision volume. 
};

}