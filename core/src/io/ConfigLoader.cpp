// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/ConfigLoader.hpp"
#include "engine/ClothMesh.hpp"
#include "physics/Solver.hpp"
#include <fstream>
#include <iostream>
#include <filesystem> 

namespace ClothSDK {

bool ConfigLoader::load(const std::string& filepath, Solver& solver, ClothMaterial& outMaterial) {
    std::cout << "[Debug] Attempting to load: " << std::filesystem::absolute(filepath) << std::endl;
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
            solver.setGravity(jsonToVector(sim["gravity"]));
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
            solver.setWind(jsonToVector(aero["wind_velocity"]));
        } else {
            solver.setWind(Eigen::Vector3d(5.0, 0.0, 0.0));
        }
        solver.setAirDensity(aero.value("air_density", 0.1));
    }

    if (data.contains("collisions")) {
        auto col = data["collisions"];

        solver.setThickness(col.value("thickness", 0.08));
    }

    return true;
}

bool ConfigLoader::save(const std::string& filepath, const Solver& solver, const ClothMaterial& material) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;

    nlohmann::json data;

    data["simulation"]["substeps"] = solver.getSubsteps();
    data["simulation"]["iterations"] = solver.getIterations();
    data["simulation"]["gravity"] = vectorToJson(solver.getGravity());

    data["material"]["density"] = material.density;
    data["material"]["compliance"]["structural"] = material.structuralCompliance;
    data["material"]["compliance"]["shear"] = material.shearCompliance;
    data["material"]["compliance"]["bending"] = material.bendingCompliance;

    data["aerodynamics"]["wind_velocity"] = vectorToJson(solver.getWind());
    data["aerodynamics"]["air_density"] = solver.getAirDensity();

    data["collisions"]["thickness"] = solver.getThickness();

    file << data.dump(4);
    file.close();
    return true;
}

Eigen::Vector3d ConfigLoader::jsonToVector(const nlohmann::json& json) {
    if (!json.is_array() || json.size() != 3)
        return Eigen::Vector3d::Zero();

    Eigen::Vector3d pos(json[0].get<double>(), json[1].get<double>(), json[2].get<double>());
    return pos;
}

nlohmann::json ConfigLoader::vectorToJson(const Eigen::Vector3d& vector) {
    return nlohmann::json{ vector.x(), vector.y(), vector.z() };
}



}