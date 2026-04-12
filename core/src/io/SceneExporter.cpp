// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/SceneExporter.hpp"
#include "engine/Cloth.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <memory>
#include <vector>

namespace Tissu {

void SceneExporter::saveScene(const std::string& filepath, const std::string& name ,Solver& solver, World& world) {
    std::ofstream file(filepath);
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);

    nlohmann::ordered_json data;

    data["version"] = "2.0";
    data["type"] = "scene"; 
    data["name"] = name;

    //TODO: Export physics into his own file?
    data["physics"] = "";

    const auto& fabrics = world.getCloths();
    saveFabrics(data, fabrics);

    const auto& colliders = world.getColliders();
    saveColliders(data, colliders);

    file << data.dump(4);
    file.close();
}

void SceneExporter::saveFabrics(nlohmann::ordered_json& data, const std::vector<std::shared_ptr<Cloth>>& fabrics) {
    data["fabrics"] = nlohmann::ordered_json::array();

    for (const auto& cloth : fabrics) {
        nlohmann::ordered_json fabric;
        fabric["name"] = cloth->getName();
        fabric["type"] = cloth->isGrid() ? "grid" : "mesh";

        if (cloth->isGrid()) {
            fabric["rows"] = cloth->getRows();
            fabric["cols"] = cloth->getCols();
            fabric["spacing"] = cloth->getSpacing();
            //TODO: Export material into his own file?
            fabric["material"] = "";
        }

        data["fabrics"].push_back(fabric);
    }
}

void SceneExporter::saveColliders(nlohmann::ordered_json& data, const std::vector<std::shared_ptr<Collider>>& colliders) {
    data["colliders"] = nlohmann::ordered_json::array();

    for (const auto& collider : colliders) {
        nlohmann::ordered_json cld;

        if (auto* plane = dynamic_cast<PlaneCollider*>(collider.get())) {
            cld["type"] = "plane";
            cld["origin"] = plane->getOrigin();
            cld["normal"] = plane->getNormal();
        }

        else if (auto* sphere = dynamic_cast<SphereCollider*>(collider.get())) {
            cld["type"] = "sphere";
            cld["center"] = sphere->getCenter();
            cld["radius"] = sphere->getRadius();
        }

        else if (auto* capsule = dynamic_cast<CapsuleCollider*>(collider.get())) {
            cld["type"] = "capsule";
            cld["start"] = capsule->getStart();
            cld["end"] = capsule->getEnd();
            cld["radius"] = capsule->getRadius();
        }

        cld["friction"] = collider->getFriction();

        data["colliders"].push_back(cld);
    }

}

}