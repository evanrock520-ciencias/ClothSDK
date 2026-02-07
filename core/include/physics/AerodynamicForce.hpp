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
#include <Eigen/Dense>

#include "physics/Force.hpp"
#include "physics/Particle.hpp"

namespace ClothSDK {

struct AeroFace {
    int a, b, c;
};

class AerodynamicForce final : public Force {
public:
    AerodynamicForce(
        const std::vector<AeroFace>& faces,
        const Eigen::Vector3d& wind,
        double airDensity
    );

    void apply(std::vector<Particle>& particles, double dt) override;

    inline void setWind(const Eigen::Vector3d& wind);
    inline const Eigen::Vector3d& getWind() const;

    inline void setAirDensity(double density);
    inline double getAirDensity() const;
    inline void setFaces(AeroFace face) { m_faces.push_back(face); }

private:
    std::vector<AeroFace> m_faces;
    Eigen::Vector3d m_wind;
    double m_airDensity;
    double m_time = 0.0;
};

}
