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