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

#include <memory>
#include <string>
#include <vector>

#include "engine/Cloth.hpp"
#include "math/Types.hpp"

struct GLFWwindow;

namespace Tissu {

class Solver;
class ClothMesh;
class World;
class GravityForce;

namespace Viewer {

class Renderer;
class Camera;

class Application {
 public:
  Application();
  ~Application();

  bool init(int width, int height, const std::string& title,
            const std::string& shaderPath);
  void run();
  void shutdown();
  void syncVisualTopology();
  int findClosestParticleToRay(const Ray& ray,
                               const std::vector<Particle>& particles);

  inline void setWorld(std::shared_ptr<World> world) { m_world = world; }
  inline void setSolver(std::shared_ptr<Solver> solver) { m_solver = solver; }
  inline void setMesh(std::shared_ptr<ClothMesh> mesh) { m_mesh = mesh; }
  void setAeroForce(std::shared_ptr<AerodynamicForce> force) {
    m_aeroForce = force;
  }
  inline void setCloth(std::shared_ptr<Cloth> cloth) {
    m_cloth = cloth;
    m_initRows = cloth->getRows();
    m_initCols = cloth->getCols();
    cloth_material = cloth->getMaterial();
  }
  inline Renderer& getRenderer() { return *m_renderer; }

 private:
  void processInput();
  void update();
  void render();
  void drawUI();
  void resetSimulation();

  GLFWwindow* m_window;
  std::shared_ptr<World> m_world;
  std::shared_ptr<Solver> m_solver;
  std::unique_ptr<Renderer> m_renderer;
  std::unique_ptr<Camera> m_camera;
  std::shared_ptr<ClothMesh> m_mesh;
  std::shared_ptr<Cloth> m_cloth;
  std::shared_ptr<ClothMaterial> cloth_material;
  std::shared_ptr<AerodynamicForce> m_aeroForce;
  std::shared_ptr<GravityForce> m_gravityForce;
  double m_deltaTime;
  double m_lastFrame;

  double m_lastX = 0.0;
  double m_lastY = 0.0;
  bool m_firstMouse = true;

  bool m_isPaused;
  bool m_isGridScene;
  int m_initRows, m_initCols;
  double m_initSpacing;

  char m_configMaterialPath[256] = "data/configs/materials/silk.json";
  char m_configPhysicsPath[256] = "data/configs/physics/realtime.json";

  std::vector<Eigen::Vector3d> m_originalPositions;
  std::vector<int> m_originalIndices;

  double m_maxRadius = 1.0;

  int m_grabbedParticleIndex = -1;
  bool m_isGrabbing = false;
  bool m_leftMouseWasPressed = false;
  double m_grabDistance = 0.0;

  float m_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  bool m_showColliders = true;
};

}  // namespace Viewer
}  // namespace Tissu