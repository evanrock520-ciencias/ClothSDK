#include "physics/Particle.hpp"

namespace ClothSDK {

Particle::Particle(const Eigen::Vector3d& pos) : m_position(pos), m_oldPosition(pos), m_acceleration(Eigen::Vector3d::Zero()), inverseMass(1.0) {}

void Particle::addForce(const Eigen::Vector3d& force) {
    m_acceleration += force * inverseMass;
}

void Particle::clearForces() {
    m_acceleration = Eigen::Vector3d::Zero();
}

void Particle::integrate(double deltaTime) {
    Eigen::Vector3d current = m_position;
    m_position = m_position + 0.99 * (m_position - m_oldPosition) + m_acceleration * (deltaTime * deltaTime);
    m_oldPosition = current;
    clearForces();
}

void Particle::setPosition(const Eigen::Vector3d& newPosition) {
    m_position = newPosition;
}

void Particle::setInverseMass(double invMass) {
    inverseMass = invMass;
}

void Particle::addMass(double mass) {
    if (inverseMass == 0.0) return;

    double currentMass = 1.0 / inverseMass;
    currentMass += mass;
    inverseMass = 1.0 / currentMass;
}

}