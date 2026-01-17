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