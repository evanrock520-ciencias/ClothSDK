// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/SceneExporter.hpp"

#include <fstream>
#include <memory>
#include <vector>

#include "engine/Cloth.hpp"
#include "io/ConfigLoader.hpp"
#include "math/Types.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/MeshCollider.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include "utils/Logger.hpp"

namespace Tissu {

void SceneExporter::saveScene(const std::string& filepath,
                              const std::string& name, Solver& solver,
                              World& world) {
  std::ofstream file(filepath);
  if (!file.is_open())
    throw std::runtime_error("Could not open file: " + filepath);

  nlohmann::ordered_json data;

  data["version"] = "2.0";
  data["type"] = "scene";
  data["name"] = name;

  data["physics"] = nlohmann::ordered_json::object();
  data["physics"]["substeps"] = solver.getSubsteps();
  data["physics"]["iterations"] = solver.getIterations();
  data["physics"]["gravity"] = ConfigLoader::vectorToJson(world.getGravity());
  data["physics"]["collision"]["thickness"] = world.getThickness();
  data["physics"]["environment"]["wind"] =
      ConfigLoader::vectorToJson(world.getWind());
  data["physics"]["environment"]["air_density"] = world.getAirDensity();

  const auto& fabrics = world.getCloths();
  saveFabrics(data, fabrics);

  const auto& colliders = world.getColliders();
  saveColliders(data, colliders);

  file << data.dump(4);
  file.close();
}

void SceneExporter::saveFabrics(
    nlohmann::ordered_json& data,
    const std::vector<std::shared_ptr<Cloth>>& fabrics) {
  data["fabrics"] = nlohmann::ordered_json::array();

  for (const auto& cloth : fabrics) {
    nlohmann::ordered_json fabric;
    fabric["name"] = cloth->getName();
    fabric["type"] = cloth->isGrid() ? "grid" : "mesh";

    if (cloth->isGrid()) {
      fabric["rows"] = cloth->getRows();
      fabric["cols"] = cloth->getCols();
      fabric["spacing"] = cloth->getSpacing();
    }

    else if (cloth->isMesh())
      fabric["path"] = cloth->getMeshPath();

    std::shared_ptr<ClothMaterial> material = cloth->getMaterial();
    fabric["material"]["density"] = material->getDensity();
    fabric["material"]["compliance"]["structural"] =
        material->getStructuralCompliance();
    fabric["material"]["compliance"]["shear"] = material->getShearCompliance();
    fabric["material"]["compliance"]["bending"] =
        material->getBendingCompliance();

    const Pin& pin = cloth->getPin();
    if (pin.getPinMode() != NONE) {
      fabric["pins"]["compliance"] = pin.getCompliance();
      fabric["pins"]["threshold"] = pin.getThreshold();

      if (pin.getPinMode() == TOP_CORNERS)
        fabric["pins"]["mode"] = "top_corners";
      else if (pin.getPinMode() == BY_HEIGHT)
        fabric["pins"]["mode"] = "by_height";
    }

    const auto& t = cloth->getTranslation();
    fabric["translation"] = {t.x(), t.y(), t.z()};
    const auto& q = cloth->getRotation();
    fabric["rotation"] = {q.x(), q.y(), q.z(), q.w()};

    data["fabrics"].push_back(fabric);
  }
}

void SceneExporter::saveColliders(
    nlohmann::ordered_json& data,
    const std::vector<std::shared_ptr<Collider>>& colliders) {
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

    else if (auto* mesh = dynamic_cast<MeshCollider*>(collider.get())) {
      cld["type"] = "mesh";
      cld["path"] = mesh->getMeshPath();
    }

    cld["friction"] = collider->getFriction();
    cld["name"] = collider->getName();

    data["colliders"].push_back(cld);
  }
}

}  // namespace Tissu
