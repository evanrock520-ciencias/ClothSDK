// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/SceneLoader.hpp"

#include <stdexcept>
#include <string>

#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "io/ConfigLoader.hpp"
#include "io/OBJLoader.hpp"
#include "math/Types.hpp"
#include "nlohmann/detail/meta/type_traits.hpp"
#include "utils/Logger.hpp"

namespace Tissu {

void SceneLoader::loadScene(const std::string& filepath, Solver& solver,
                            World& world) {
  std::ifstream file(filepath);
  if (!file.is_open())
    throw std::runtime_error("Could not open file: " + filepath);

  nlohmann::json data;
  try {
    data = nlohmann::json::parse(file);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("Invalid JSON in " + filepath + ": " + e.what());
  }

  if (data["type"] != "scene")
    throw std::invalid_argument("The given JSON is not a valid scene.");

  if (data.contains("physics")) {
    const auto& physicsData = data.at("physics");
    if (physicsData.is_string())
      ConfigLoader::loadPhysics(physicsData.get<std::string>(), solver, world);
    else if (physicsData.is_object())
      ConfigLoader::loadPhysicsFromJson(physicsData, solver, world);
  }

  for (const auto& fabricData : data.at("fabrics")) {
    std::string name = fabricData.value("name", "cloth");
    auto material = std::make_shared<ClothMaterial>();
    auto cloth = std::make_shared<Cloth>(name, material);

    loadFabric(fabricData, *cloth, solver);

    world.addCloth(cloth);
  }

  if (data.contains("colliders")) {
    for (const auto& colliderData : data.at("colliders"))
      loadCollider(colliderData, world);
  }
}

void SceneLoader::loadFabric(const nlohmann::json& fabric, Cloth& outCloth,
                             Solver& solver) {
  std::string type = fabric.value("type", "grid");
  std::string name = fabric.value("name", "default");

  if (fabric.contains("material")) {
    const auto& materialData = fabric.at("material");
    if (materialData.is_string())
      ConfigLoader::loadMaterial(materialData.get<std::string>(),
                                 *outCloth.getMaterial());
    else if (materialData.is_object())
      ConfigLoader::loadMaterialFromJson(materialData, *outCloth.getMaterial());
  } else
    ConfigLoader::loadMaterial("data/configs/materials/silk.json",
                               *outCloth.getMaterial());

  outCloth.setName(name);

  ClothMesh mesh;

  if (type == "grid") {
    int rows = fabric.value("rows", 10);
    int cols = fabric.value("cols", 10);
    double spacing = fabric.value("spacing", 0.1);

    mesh.initGrid(rows, cols, spacing, outCloth, solver);
  } else if (type == "mesh") {
    std::string path = fabric.value("path", "");

    std::vector<Eigen::Vector3d> positions;
    std::vector<int> indices;

    if (!OBJLoader::load(path, positions, indices))
      throw std::runtime_error("SceneLoader: could not load OBJ: " + path);

    mesh.buildFromMesh(positions, indices, outCloth, solver, path);
  }

  if (fabric.contains("pins")) {
    auto pins = fabric.at("pins");
    auto mode = pins.value("mode", "none");
    auto compliance = pins.value("compliance", 0.0);
    auto threshold = pins.value("threshold", 0.1);

    Logger::info("Mode: " + mode);
    if (mode == "top_corners")
      outCloth.setPin(Pin(TOP_CORNERS, compliance, threshold));
    else if (mode == "by_height")
      outCloth.setPin(Pin(BY_HEIGHT, compliance, threshold));
  }
}

void SceneLoader::loadCollider(const nlohmann::json& collider, World& world) {
  std::string type = collider.value("type", "plane");
  double friction = collider.value("friction", 0.2);

  if (type == "plane") {
    Eigen::Vector3d origin = ConfigLoader::jsonToVector(
        collider.value("origin", nlohmann::json{0.0, 0.0, 0.0}));
    Eigen::Vector3d normal = ConfigLoader::jsonToVector(
        collider.value("normal", nlohmann::json{0.0, 1.0, 0.0}));

    world.addPlaneCollider(origin, normal, friction);
  }

  else if (type == "sphere") {
    Eigen::Vector3d center = ConfigLoader::jsonToVector(
        collider.value("center", nlohmann::json{0.0, 0.0, 0.0}));
    double radius = collider.value("radius", 1.0);

    world.addSphereCollider(center, radius, friction);
  }

  else if (type == "capsule") {
    Eigen::Vector3d start = ConfigLoader::jsonToVector(
        collider.value("start", nlohmann::json{0.0, 0.0, 0.0}));
    Eigen::Vector3d end = ConfigLoader::jsonToVector(
        collider.value("end", nlohmann::json{0.0, 4.0, 0.0}));
    double radius = collider.value("radius", 1.0);

    world.addCapsuleCollider(start, end, radius, friction);
  }

  else
    throw std::invalid_argument("Unknown collider " + type);
}

}  // namespace Tissu
