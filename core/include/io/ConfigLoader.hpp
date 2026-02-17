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

#include "engine/World.hpp"
#include "math/Types.hpp"
#include <string>
#include <nlohmann/json.hpp>
#include <Eigen/Dense>
#include <fstream>

namespace ClothSDK {

class Solver;
class ClothMesh;

/**
 * @class ConfigLoader
 * @brief Static utility class for managing configuration persistence.
 * 
 * This class handles the conversion between SDK objects 
 * and JSON files.
 */

class ConfigLoader {
public:

    /**
     * @brief Loads a configuration from a JSON file.
     * 
     * @param filepath Path to the configuration file.
     * @param solver The solver instance to configure.
     * @param world Reference to the physics world.
     * @param outMaterial The material object that will be updated with loaded data.
     * @return true if loading was successful, false otherwise.
     */
    static bool load(const std::string& filepath, Solver& solver, World& world, ClothMaterial& outMaterial);

    /**
     * @brief Saves the current material configuration to a JSON file.
     *
     * @param filepath The destination path where the config file will be created.
     * @param solver The solver instance whose parameters need to be saved.
     * @param world The physics world context to include in the configuration.
     * @param material The material properties to be serialized.
     * @return true if the file was written successfully, false if the path is invalid or inaccessible.
     */
    static bool save(const std::string& filepath, const Solver& solver, const World& world, const ClothMaterial& material);

private:

    private:
    /**
     * @brief Converts a JSON object into an Eigen vector.
     * @param json JSON object containing coordinates 
     * @return Eigen::Vector3d Resulting vector representation.
     */
    static Eigen::Vector3d jsonToVector(const nlohmann::json& json);    
    
    /**
     * @brief Converts an Eigen vector into a JSON object.
     * 
     * @param vector The Eigen::Vector3d to be converted.
     * @return nlohmann::json A JSON object representing the vector's coordinates.
     */
    static nlohmann::json vectorToJson(const Eigen::Vector3d& vector);      
    
};

}