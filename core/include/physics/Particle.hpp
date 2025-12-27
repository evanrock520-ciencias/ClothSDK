#pragma once

#include <Eigen/Dense>

namespace ClothSDK {

class Particle {
public:
    Particle(const Eigen::Vector3d& initialPos);

    void addForce(const Eigen::Vector3d& force);
    void clearForces();
    void integrate(double deltaTime);

    inline const Eigen::Vector3d& getPosition() const { return m_position; }
    inline const Eigen::Vector3d& getAcceleration() const { return m_acceleration; }
    inline const Eigen::Vector3d& getOldPosition() const { return m_oldPosition; }
    inline const double getInverseMass() const { return inverseMass; }

private:
    Eigen::Vector3d m_position;
    Eigen::Vector3d m_oldPosition;
    Eigen::Vector3d m_acceleration;
    double          inverseMass;
};

}