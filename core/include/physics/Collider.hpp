#pragma once

#include <vector>

namespace ClothSDK {

class Particle;

class Collider {
public:
    virtual ~Collider() = default;

    virtual void resolve(std::vector<Particle>& particles) = 0;

    void setFriction(double friction) { m_friction = friction; }
    double getFriction() const { return m_friction; }

protected:
    double m_friction = 0.5;
};

}