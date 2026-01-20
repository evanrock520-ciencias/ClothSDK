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

#include "physics/Constraint.hpp"
namespace ClothSDK {

class PinConstraint : public Constraint {
public:
    PinConstraint(int particleId, const Eigen::Vector3d& pinPosition, double compliance);
    void solve(std::vector<Particle>& particles, double dt) override;

    inline void setPinPosition(const Eigen::Vector3d& newPos) { m_pinPos = newPos; }

private:
    int m_particleId;     
    Eigen::Vector3d m_pinPos;
};

}