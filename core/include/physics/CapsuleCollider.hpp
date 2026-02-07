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

#include "physics/Collider.hpp"

namespace ClothSDK {

class CapsuleCollider : public Collider {
public:
    CapsuleCollider(double radius, const Eigen::Vector3d& start, const Eigen::Vector3d& end, double friction);

    void resolve(std::vector<Particle>& particles, double dt, double thickness) override;

    inline double getRadius() const { return m_radius; }
    inline const Eigen::Vector3d& getStart() const { return m_start; }
    inline const Eigen::Vector3d& getEnd() const { return m_end; }

private:
    double m_radius;
    Eigen::Vector3d m_start;
    Eigen::Vector3d m_end;
};

}