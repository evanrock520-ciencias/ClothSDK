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

#include "Constraint.hpp"
#include "Particle.hpp"
#include <vector>

namespace ClothSDK {

/**
 * @class DistanceConstraint
 * @brief Implementation of a linear constraint using XPBD.
 * 
 * The constraint function is defined as:
 * @f[
 * C(\mathbf{p}_1, \mathbf{p}_2) = |\mathbf{p}_1 - \mathbf{p}_2| - L_{rest}
 * @f]
 * where @f$ L_{rest} @f$ is the natural length of the connection.
 */
class DistanceConstraint : public Constraint {
public:
    /**
     * @brief Constructs a distance constraint between two particles.
     * 
     * @param idA Index of the first particle in the solver buffer.
     * @param idB Index of the second particle in the solver buffer.
     * @param restLength The target distance the constraint tries to maintain.
     * @param compliance Physical compliance (inverse of the stiffness), measured in m/N.
     */
    DistanceConstraint(int idA, int idB, double restLength, double compliance);

    /**
     * @brief Solves the constraint by updating particle positions and the Lagrange multiplier.
     * Following the XPBD formulation, the Lagrange multiplier increment @f$ \Delta \lambda @f$ is:
     * @f[
     * \Delta \lambda = \frac{-C(\mathbf{p}) - \tilde{\alpha} \lambda}{\sum w_i |\nabla C_i|^2 + \tilde{\alpha}}
     * @f]
     * where @f$ \tilde{\alpha} = \frac{\alpha}{\Delta t^2} @f$ is the time-step-corrected compliance.
     *
     * @param particles Reference to the solver's particle buffer.
     * @param dt Current substep time delta.
     */
    void solve(std::vector<Particle>& particles, double dt) override;

private:
    int m_idA;              ///< Index of the first particle.
    int m_idB;              ///< Index of the second particle.
    double m_restLength;    ///< Natural length of the constraint.
    double m_compliance;    ///< Physical compliance @f$ \alpha @f$.
};

}