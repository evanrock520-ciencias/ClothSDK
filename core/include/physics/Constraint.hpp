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