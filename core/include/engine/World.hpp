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

#include <memory>
#include <vector>
#include <string>
#include <Eigen/Dense>

namespace ClothSDK {

class Cloth;
class Collider;
class Force;

class World {
public:
    World();

    void addCloth(std::shared_ptr<Cloth> cloth);
    void addCollider(std::shared_ptr<Collider> collider);
    void addForce(std::shared_ptr<Force> force);
    void clear();

    void addPlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction);
    void addSphereCollider(const Eigen::Vector3d& center, double radius, double friction);

    inline void setGravity(const Eigen::Vector3d& gravity) { m_gravity = gravity; }
    inline void setWind(const Eigen::Vector3d& wind) { m_wind = wind; }
    inline void setAirDensity(double density) { m_airDensity = density; }
    inline void setThickness(double thickness) { m_thickness = thickness; }
    
    inline const Eigen::Vector3d& getGravity() const { return m_gravity; }
    inline const Eigen::Vector3d& getWind() const { return m_wind; }
    inline double getAirDensity() const { return m_airDensity; }
    inline double getThickness() const { return m_thickness; }

    inline const std::vector<std::shared_ptr<Cloth>>& getCloths() const { return m_cloths; }
    inline const std::vector<std::shared_ptr<Collider>>& getColliders() const { return m_colliders; }
    inline const std::vector<std::shared_ptr<Force>>& getForces() const { return m_forces; }

private:
    std::vector<std::shared_ptr<Cloth>> m_cloths;
    std::vector<std::shared_ptr<Collider>> m_colliders;
    std::vector<std::shared_ptr<Force>> m_forces;

    Eigen::Vector3d m_gravity;
    Eigen::Vector3d m_wind;
    double m_airDensity;
    double m_thickness;
};

} 