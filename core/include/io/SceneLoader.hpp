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

#include <string>

#include "engine/World.hpp"
#include "math/Types.hpp"
#include "nlohmann/json_fwd.hpp"
#include "physics/Solver.hpp"

namespace Tissu {

struct SceneHeader {
    float version;
    std::string name;
    std::string physics_preset;

    struct FabricInfo {
        std::string name;
        std::string type;
        int rows = 0, cols = 0;
        float spacing = 0.0f;
        Eigen::Vector3d translation;
        Eigen::Quaterniond rotation;
        std::string source;
        std::string material;
        std::string pin_mode;
    };

    struct ColliderInfo {
        std::string name;
        std::string type;
        std::string summary;
    };

    std::vector<FabricInfo> fabrics;
    std::vector<ColliderInfo> colliders;
};

class SceneLoader {
public:
    static void loadScene(const std::string& filepath, Solver& solver,
                          World& world);

    static SceneHeader getSceneHeader(const std::string& filepath);

private:
    static void loadFabric(const nlohmann::json& fabricData, Cloth& outCloth,
                           Solver& solver);
    static void loadCollider(const nlohmann::json& collider, World& world);
};

} // namespace Tissu
