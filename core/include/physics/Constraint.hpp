#pragma once

#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

/**
 * @class Constraint
 * @brief Base interface for all physical constraints in the XPBD simulation.
 * 
 * A constraint represents a geometric or physical rule that must be satisfied
 * by particles. In the context of XPBD constraints are treated as potential energy 
 * functions that yield position corrections and accumulate Lagrange multipliers.
 */
class Constraint {
public:
    /**
     * @brief Construct the constraint with zeroed internal state.
     * 
     */
    Constraint() : m_lambda(0.0), m_compliance(0.0) {}
    
    /**
     * @brief Destroy the constraint to ensure correct cleanup of derived constraints.
     * 
     */
    virtual ~Constraint() = default;

    /**
     * @brief Virtual method to resolve the constraint.
     *
     * Derivated classes must implement the specific XPBD projection logic here.
     *
     * 
     * @param particles Reference to the global particle buffer.
     * @param dt The current substep time delta.
     */
    virtual void solve(std::vector<Particle>& particles, double dt) = 0;

    /**
     * @brief Resets the accumulated Lagrange multiplier.
     * 
     */
    virtual void resetLambda() { m_lambda = 0.0; }

protected:
    /**
     * @brief Accumulated Lagrange multiplier for the current substep.
     * 
     */
    double m_lambda;  
    
    /**
     * @brief Physical compliance of the constraint.
     * 
     */
    double m_compliance;    
};

}