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

#include "math/Types.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <Eigen/Dense>
#include <fstream>

namespace ClothSDK {

class Solver;
class ClothMesh;

class ConfigLoader {
public:

    static bool load(const std::string& filepath, Solver& solver, ClothMaterial& outMaterial);
    static bool save(const std::string& filepath, const Solver& solver, const ClothMaterial& material);

private:

    static Eigen::Vector3d jsonToVector(const nlohmann::json& json);
    static nlohmann::json vectorToJson(const Eigen::Vector3d& vector);
    
};

}