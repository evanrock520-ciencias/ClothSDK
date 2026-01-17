#pragma once

#include "Collider.hpp"
#include <Eigen/Dense>

namespace ClothSDK {

/**
 * @class PlaneCollider
 * @brief Implementation of an infinite plane collision volume.
 *
 * This collider defines a half-space using an origin point and a constant normal vector.
 * It is typically used for floors, walls, or static flat surfaces. Particles penetrating 
 * the plane are projected back to the surface along the fixed normal.
 */
class PlaneCollider : public Collider {
public:
    /**
     * @brief Constructs a new Plane Collider.
     * 
     * @param origin Any point residing on the collision plane.
     * @param normal A vector defining the collision side of the plane.
     * @param friction The friction coefficient [0.0 - 1.0] for tangential damping.
     */
    PlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction);
    
    /**
     * @brief Projects penetrating particles onto the plane's surface.
     * 
     * The method calculates the signed distance of each particle from the plane. 
     * If the distance is less than the collision thickness, the particle is 
     * translated along the normal and its implicit velocity is damped.
     * 
     * @param particles Reference to the global particle buffer.
     * @param dt Current substep time delta.
     */
    void resolve(std::vector<Particle>& particles, double dt, double thickness);

private:
    Eigen::Vector3d m_origin;   ///< World-space coordinate of a point in the plane.  
    Eigen::Vector3d m_normal;   ///< Normalized vector defining the surface orientation.
};

}