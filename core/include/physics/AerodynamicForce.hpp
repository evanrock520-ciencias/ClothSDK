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

#include <Eigen/Dense>
#include <vector>

#include "physics/Force.hpp"
#include "physics/Particle.hpp"

namespace Tissu {

/**
 * @struct AeroFace
 * @brief Triangle face used for aerodynamic force calculations.
 *
 */
struct AeroFace {
  int a, b, c;
};

/**
 * @class AerodynamicForce
 * @brief Computes and applies aerodynamic pressure forces to cloth faces.
 *
 * Models wind interaction using per-face pressure based on the relative
 * velocity between the wind and the face's surface normal.
 */
class AerodynamicForce final : public Force {
 public:
  AerodynamicForce(const std::vector<AeroFace>& faces,
                   const Eigen::Vector3d& wind, double airDensity);

  /**
   * @brief Computes aerodynamic pressure and accumulates in into each particle.
   *
   * The force applied to each face is:
   * @f[
   * \mathbf{F} = -\frac{1}{2} \rho \|\mathbf{v}_{rel}\| \cdot (\mathbf{v}_{rel}
   * \cdot \hat{n}) \cdot A \cdot \hat{n}
   * @f]
   * where @f$ \rho @f$ is the air density, @f$ \mathbf{v}_{rel} @f$ is the
   * relative velocity between the face and the wind, @f$ \hat{n} @f$ is the
   * face normal, and @f$ A @f$ is the triangle area. The resulting force is
   * distributed equally across the three vertices of each face.
   *
   *
   * @param particles Reference to the solver's global particle buffer.
   * @param dt Current substep time delta.
   */
  void apply(std::vector<Particle>& particles, double dt) override;

  inline const Eigen::Vector3d& getWind() const { return m_wind; }
  inline double getAirDensity() const { return m_airDensity; }

  inline void setWind(const Eigen::Vector3d& wind) { m_wind = wind; }
  inline void setAirDensity(double density) { m_airDensity = density; }
  inline void setFaces(AeroFace face) { m_faces.push_back(face); }

 private:
  std::vector<AeroFace> m_faces;  ///< Aerodynamic triangle faces.
  Eigen::Vector3d m_wind;  ///< Base wind velocity vector in world space (m/s).
  double m_airDensity;  ///< Air density used in pressure calculation (kg/m³).
  double m_time =
      0.0;  ///< Accumulated simulation time, used for gust oscillation.
};

}  // namespace Tissu
