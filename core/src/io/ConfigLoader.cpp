// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/ConfigLoader.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"     
#include "nlohmann/json_fwd.hpp"
#include "physics/Solver.hpp"
#include <fstream>
#include <iostream>
#include <filesystem> 
#include <stdexcept>
#include <string>

namespace Tissu {

void ConfigLoader::loadMaterial(const std::string& filepath, ClothMaterial& outMaterial) {
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Invalid JSON in " + filepath + ": " + e.what());
    }

    loadMaterialFromJson(data, outMaterial);
}

void ConfigLoader::loadMaterialFromJson(const nlohmann::json& data, ClothMaterial& outMaterial) {
    if (data.contains("type") && data["type"] != "material") 
        throw std::invalid_argument("The given JSON is not a material preset.");

    auto comp = data.at("compliance");

    outMaterial.setDensity(data.value("density", 0.1));
    outMaterial.setStructuralCompliance(comp.value("structural", 1e-6));
    outMaterial.setShearCompliance(comp.value("shear", 1e-6));
    outMaterial.setBendingCompliance(comp.value("bending", 1e-4));
}

void ConfigLoader::loadPhysics(const std::string& filepath, Solver& solver, World& world) {
    std::ifstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        throw std::runtime_error("Invalid JSON in " + filepath + ": " + e.what());
    }

    loadPhysicsFromJson(data, solver, world);
}

void ConfigLoader::loadPhysicsFromJson(const nlohmann::json& data, Solver& solver, World& world) {
    if (data.contains("type") && data["type"] != "physics") 
        throw std::invalid_argument("The given JSON is not a physics preset.");

    solver.setSubsteps(data.value("substeps", 10));
    solver.setIterations(data.value("iterations", 2));

    if (data.contains("gravity"))
        world.setGravity(jsonToVector(data.at("gravity")));

    if (data.contains("collision"))
        world.setThickness(data["collision"].value("thickness", 0.02));

    if (data.contains("environment")) {
        auto env = data.at("environment");
        if (env.contains("wind"))
            world.setWind(jsonToVector(env.at("wind")));
        if (env.contains("air_density"))
            world.setAirDensity(env.value("air_density", 0.1));
    }
}

void ConfigLoader::saveMaterial(const std::string& filepath, const ClothMaterial& material, const std::string& name) {
    std::ofstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    nlohmann::ordered_json data;

    data["version"] = "2.0";
    data["type"] = "material";
    data["name"] = name;
    data["density"] = material.getDensity();
    data["compliance"]["structural"] = material.getStructuralCompliance();
    data["compliance"]["shear"] = material.getShearCompliance();
    data["compliance"]["bending"] = material.getBendingCompliance();

    file << data.dump(4);
    file.close();
}

void ConfigLoader::savePhysics(const std::string &filepath, const Solver &solver, const World &world, const std::string &name){
    std::ofstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    nlohmann::ordered_json data;

    data["version"] = "2.0";
    data["type"] = "physics";
    data["name"] = name;
    data["substeps"] = solver.getSubsteps();
    data["iterations"] = solver.getIterations();
    data["gravity"] = vectorToJson(world.getGravity());
    data["collision"]["thickness"] = world.getThickness();
    data["environment"]["wind"] = vectorToJson(world.getWind());
    data["environment"]["air_density"] = world.getAirDensity();

    file << data.dump(4);
    file.close();
}

Eigen::Vector3d ConfigLoader::jsonToVector(const nlohmann::json& json) {
    if (!json.is_array() || json.size() != 3)
        return Eigen::Vector3d::Zero();

    return Eigen::Vector3d(json[0].get<double>(), json[1].get<double>(), json[2].get<double>());
}

nlohmann::json ConfigLoader::vectorToJson(const Eigen::Vector3d& vector) {
    return nlohmann::json{ vector.x(), vector.y(), vector.z() };
}

} 