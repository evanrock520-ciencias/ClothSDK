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

#include "engine/Cloth.hpp"
#include "engine/World.hpp"
#include "physics/Collider.hpp"
#include "physics/Solver.hpp"
#include <memory>
#include <nlohmann/json.hpp> 
#include <vector>

namespace Tissu {

class SceneExporter {
public:
    static void saveScene(const std::string& filepath, const std::string& name, Solver& solver, World& world);
private:
    static void saveFabrics(nlohmann::ordered_json& data, const std::vector<std::shared_ptr<Cloth>>& fabrics);
    static void saveColliders(nlohmann::ordered_json& data, const std::vector<std::shared_ptr<Collider>>& colliders);
};

}