#pragma once

#include "Collider.hpp"
#include <Eigen/Dense>

namespace ClothSDK {

class PlaneCollider : public Collider {
public:
    PlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction);
    
    void resolve(std::vector<Particle>& particles, double dt);

private:
    Eigen::Vector3d m_origin;
    Eigen::Vector3d m_normal;
};

}