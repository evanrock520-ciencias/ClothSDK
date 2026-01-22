// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/ConfigLoader.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"     
#include "physics/Solver.hpp"
#include <fstream>
#include <iostream>
#include <filesystem> 

namespace ClothSDK {

bool ConfigLoader::load(const std::string& filepath, Solver& solver, World& world, ClothMaterial& outMaterial) {
    std::cout << "[Debug] Attempting to load config: " << std::filesystem::absolute(filepath) << std::endl;
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(file);
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return false;
    } 

    if (data.contains("simulation")) {
        auto sim = data["simulation"];
        solver.setSubsteps(sim.value("substeps", 10));
        solver.setIterations(sim.value("iterations", 5));

        if (sim.contains("gravity")) {
            world.setGravity(jsonToVector(sim["gravity"]));
        }
    }

    if (data.contains("material")) {
        auto mat = data["material"];
        auto comp = mat.value("compliance", nlohmann::json::object());

        outMaterial.density = mat.value("density", 0.1);
        outMaterial.structuralCompliance = comp.value("structural", 1e-6);
        outMaterial.shearCompliance = comp.value("shear", 1e-6);
        outMaterial.bendingCompliance = comp.value("bending", 1e-4);
    }

    if (data.contains("aerodynamics")) {
        auto aero = data["aerodynamics"];

        if (aero.contains("wind_velocity")) {
            world.setWind(jsonToVector(aero["wind_velocity"]));
        } else {
            world.setWind(Eigen::Vector3d(5.0, 0.0, 0.0));
        }
        world.setAirDensity(aero.value("air_density", 0.1));
    }

    if (data.contains("collisions")) {
        auto col = data["collisions"];
        world.setThickness(col.value("thickness", 0.08));
    }

    return true;
}

bool ConfigLoader::save(const std::string& filepath, const Solver& solver, const World& world, const ClothMaterial& material) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    nlohmann::json data;

    data["simulation"]["substeps"] = solver.getSubsteps();
    data["simulation"]["iterations"] = solver.getIterations();
    
    data["simulation"]["gravity"] = vectorToJson(world.getGravity());
    data["aerodynamics"]["wind_velocity"] = vectorToJson(world.getWind());
    data["aerodynamics"]["air_density"] = world.getAirDensity();
    data["collisions"]["thickness"] = world.getThickness();

    // Datos del Material
    data["material"]["density"] = material.density;
    data["material"]["compliance"]["structural"] = material.structuralCompliance;
    data["material"]["compliance"]["shear"] = material.shearCompliance;
    data["material"]["compliance"]["bending"] = material.bendingCompliance;

    file << data.dump(4);
    file.close();
    return true;
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