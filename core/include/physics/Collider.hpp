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

#include <vector>

namespace ClothSDK {

class Particle;

/**
 * @class Collider
 * @brief Base interface for all geometric collision objects.
 * 
 * Colliders in this engine follow the Position Based Dynamics (PBD) approach. 
 * Instead of calculating complex contact forces, they project penetrating 
 * particles back to the surface of the object and modify their implicit 
 * velocity via friction.
 */
class Collider {
public:
    /**
     * @brief Destroy the collider for safe cleanup.
     * 
     */
    virtual ~Collider() = default;

    /**
     * @brief Detects and resolves interpenetration between particles and the collider volume.
     * 
     * Derived classes must implement the specific geometry projection logic. 
     *
     * @param particles Reference to the global particle buffer.
     * @param dt Current substep time delta. Required for kinematic friction calculations.
     */
    virtual void resolve(std::vector<Particle>& particles, double dt, double thickness) = 0;

    /**
     * @brief Configures the surface friction coefficient.
     * 
     * @param friction Friction value in the range [0.0, 1.0]
     */
    void setFriction(double friction) { m_friction = friction; }

    /** @return The current surface friction coefficient. */
    inline double getFriction() const { return m_friction; }

protected:
    /**
     * @brief Tangential friction coefficient used during collision response.
     * 
     */
    double m_friction = 0.5;
};

}