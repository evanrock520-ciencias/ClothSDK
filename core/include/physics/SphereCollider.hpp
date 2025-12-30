#pragma once

#include "Collider.hpp"
#include <Eigen/Dense>

namespace ClothSDK {

class SphereCollider : public Collider {
public:
    SphereCollider(const Eigen::Vector3d& center, double radius, double friction);
    void resolve(std::vector<Particle>& particles, double dt);

private:
    Eigen::Vector3d m_center;
    double m_radius;
};

}