#pragma once

#include <Eigen/Dense>

namespace ClothSDK {

/**
 * @class Particle
 * @brief Represents an atomic mass point in the physical simulation.
 * 
 * This particle is the fundamental building block for the engine.
 */
class Particle {
public:
    /**
     * @brief Constructs a new Particle object at a specific location on the 3D world space.
     * 
     * @param initialPos Initial world-space position.
     */
    Particle(const Eigen::Vector3d& initialPos);

    /**
     * @brief Accumulates an external force into the particle's state.
     * 
     * @param force Force vector in Newtons.
     */
    void addForce(const Eigen::Vector3d& force);

    /**
     * @brief Add real mass to the particle and update its inverse mass.
     * 
     * @param mass Amount of mass in kg to add to the current value.
     */
    void addMass(double mass);

    /**
     * @brief Resets the acceleration acumulator to zero.
     * 
     */
    void clearForces();

    /**
     * @brief Updates the particle's position using the Verlet integration scheme.
     * 
     * @param deltaTime The fixed time step for the current update.
     */
    void integrate(double deltaTime);

    /**
     * @brief Sets the particle's current position.
     * 
     * @param newPosition The new point in world space.
     */
    void setPosition(const Eigen::Vector3d& newPosition);

    /**
     * @brief Set the inverse mass of the particle.
     * 
     * @param invMass The inverse mass value.
     */
    void setInverseMass(double invMass);

    /**
     * @brief Set the particle's old position.
     * 
     * @param newOldPosition The new point in the world space for the previous state.
     */
    void setOldPosition(const Eigen::Vector3d& newOldPosition);

    /** @return Constant reference to the current position vector. */
    inline const Eigen::Vector3d& getPosition() const { return m_position; }

    /** @return Constant reference to the accumulated acceleration vector. */
    inline const Eigen::Vector3d& getAcceleration() const { return m_acceleration; }

    /** @return Constant reference to the previous step's position vector. */
    inline const Eigen::Vector3d& getOldPosition() const { return m_oldPosition; }

    /** @return The current inverse mass value. */
    inline const double getInverseMass() const { return inverseMass; }

private:
    Eigen::Vector3d m_position;     ///< Current position in 3D world space.
    Eigen::Vector3d m_oldPosition;  ///< Position from the previous step.
    Eigen::Vector3d m_acceleration; ///< Force accumulator converted to acceleration.
    double inverseMass;             ///< Inverse mass.
};

}