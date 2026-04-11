#pragma once

#include "engine/World.hpp"
#include "math/Types.hpp"
#include "nlohmann/json_fwd.hpp"
#include "physics/Solver.hpp"
#include <string>

namespace Tissu {

class SceneLoader {
public:
    static void loadScene(const std::string& filepath, Solver& solver, World& world);
private:
    static void loadFabric(const nlohmann::json& fabricData, Cloth& outCloth, Solver& solver);
    static void loadCollider(const nlohmann::json& collider, World& world);
};

}