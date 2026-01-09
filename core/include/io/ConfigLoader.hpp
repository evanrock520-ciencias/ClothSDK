#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <Eigen/Dense>
#include <fstream>

namespace ClothSDK {

class Solver;
class ClothMesh;

class ConfigLoader {
public:

    static bool load(const std::string& filepath, Solver& solver, ClothMesh& mesh);

    static bool save(const std::string& filepath, const Solver& solver, ClothMesh& mesh);

private:

    static Eigen::Vector3d jsonToVector(const nlohmann::json& json);

    static nlohmann::json vectorToJson(const Eigen::Vector3d& vector);
};

}