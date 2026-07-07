// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "Application.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <Eigen/Dense>
#include <memory>
#include <string>
#include <vector>

#include "Camera.hpp"
#include "Renderer.hpp"
#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "io/ConfigLoader.hpp"
#include "io/OBJExporter.hpp"
#include "io/SceneExporter.hpp"
#include "io/StateSerializer.hpp"
#include "math/Types.hpp"
#include "physics/GravityForce.hpp"
#include "physics/Particle.hpp"
#include "physics/MeshCollider.hpp"
#include "physics/Solver.hpp"
#include "utils/Logger.hpp"

extern IMGUI_IMPL_API void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window,
                                                            double x, double y);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_MouseButtonCallback(
    GLFWwindow* window, int button, int action, int mods);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window,
                                                         double xoffset,
                                                         double yoffset);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window,
                                                      int key, int scancode,
                                                      int action, int mods);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_CharCallback(GLFWwindow* window,
                                                       unsigned int c);

namespace Tissu {
namespace Viewer {

Application::Application()
    : m_window(nullptr),
      m_solver(nullptr),
      m_renderer(nullptr),
      m_camera(nullptr),
      m_deltaTime(0.0),
      m_lastFrame(0.0),
      m_isPaused(false) {
  m_world = std::make_shared<World>();
  m_solver = std::make_shared<Solver>();
  m_isGridScene = true;
  m_initRows = 40;
  m_initCols = 40;
  m_initSpacing = 0.1;
  auto defaultMat = std::make_shared<ClothMaterial>();
  m_cloth = std::make_shared<Cloth>("MainCloth", defaultMat);
}

Application::~Application() = default;

bool Application::init(int width, int height, const std::string& title,
                       const std::string& shaderPath) {
  if (!glfwInit()) {
    Logger::error("Failed to initialize GLFW");
    return false;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
  if (!m_window) {
    Logger::error("Failed to create GLFW window");
    glfwTerminate();
    return false;
  }

  glfwMakeContextCurrent(m_window);
  glfwSetWindowUserPointer(m_window, this);
  glfwSwapInterval(1);

  if (!gladLoadGL(glfwGetProcAddress)) {
    Logger::error("Failed to initialize GLAD");
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();

  float fontSize = 24.0f;
  float scale = 2.0f;
  io.Fonts->AddFontDefault();
  ImGui::GetStyle().ScaleAllSizes(scale);
  io.FontGlobalScale = 1.0f;

  ImGui_ImplGlfw_InitForOpenGL(m_window, false);
  ImGui_ImplOpenGL3_Init("#version 330");

  glfwSetCursorPosCallback(
      m_window, [](GLFWwindow* window, double xpos, double ypos) {
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        if (ImGui::GetIO().WantCaptureMouse) return;

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app->m_firstMouse) {
          app->m_lastX = xpos;
          app->m_lastY = ypos;
          app->m_firstMouse = false;
        }

        float xoffset = static_cast<float>(xpos - app->m_lastX);
        float yoffset = static_cast<float>(app->m_lastY - ypos);

        app->m_lastX = xpos;
        app->m_lastY = ypos;

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
          app->m_camera->handleMouse(xoffset, yoffset);
        }
      });

  glfwSetMouseButtonCallback(
      m_window, [](GLFWwindow* window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
      });

  glfwSetScrollCallback(
      m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

        if (ImGui::GetIO().WantCaptureMouse) return;

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->m_camera->handleZoom(static_cast<float>(yoffset));
      });

  glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode,
                                  int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);
  });

  glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(window, c);
  });

  glfwSetFramebufferSizeCallback(
      m_window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app->m_camera && height > 0) {
          app->m_camera->setAspectRatio(static_cast<float>(width) /
                                        static_cast<float>(height));
        }
      });

  if (!m_world) m_world = std::make_shared<World>();
  if (!m_solver) m_solver = std::make_shared<Solver>();
  if (!m_mesh) m_mesh = std::make_shared<ClothMesh>();

  m_gravityForce =
      std::make_shared<GravityForce>(Eigen::Vector3d(0.0, -9.81, 0.0));
  m_world->addForce(m_gravityForce);

  m_renderer = std::make_unique<Renderer>();
  m_renderer->setShaderPath(shaderPath);

  if (!m_renderer->init()) {
    Logger::error("Failed to initialize Renderer with shader path: " +
                  shaderPath);
    return false;
  }

  int bufferWidth, bufferHeight;
  glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);

  m_camera = std::make_unique<Camera>(Eigen::Vector3f(1.0f, 1.0f, 5.0f),
                                      Eigen::Vector3f(1.0f, 1.0f, 0.0f));

  if (bufferHeight > 0) {
    m_camera->setAspectRatio(static_cast<float>(bufferWidth) /
                             static_cast<float>(bufferHeight));
    glViewport(0, 0, bufferWidth, bufferHeight);
  }

  return true;
}

void Application::run() {
  m_lastFrame = glfwGetTime();

  while (!glfwWindowShouldClose(m_window)) {
    double currentFrame = glfwGetTime();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
    if (m_deltaTime > 0.05) m_deltaTime = 0.05;

    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawUI();
    processInput();

    update();

    render();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(m_window);
  }
}

void Application::processInput() {
  if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_window, true);

  static bool spaceWasPressed = false;
  bool spaceIsPressed = (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS);

  if (spaceIsPressed && !spaceWasPressed) {
    m_isPaused = !m_isPaused;
    Logger::info(m_isPaused ? "Simulation Paused" : "Simulation Resumed");
  }
  spaceWasPressed = spaceIsPressed;

  static bool sWasPressed = false;
  bool sIsPressed = (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS);

  if (sIsPressed && !sWasPressed) {
    int frame = m_solver->getCurrentFrame();
    const std::string& cloth_name = m_cloth->getName();
    std::string name =
        "data/snapshots/" + cloth_name + "On" + std::to_string(frame) + ".obj";
    OBJExporter::exportOBJ(name, *m_cloth, *m_solver);
    Logger::info("Taking snapshot: " + name);
  }
  sWasPressed = sIsPressed;

  static bool gWasPressed = false;
  bool gIsPressed = (glfwGetKey(m_window, GLFW_KEY_G) == GLFW_PRESS);

  if (gIsPressed && !gWasPressed) {
    int frame = m_solver->getCurrentFrame();
    std::string name = "data/states/frame" + std::to_string(frame) + ".tissu";
    StateSerializer::save(name, *m_solver, *m_world);
    Logger::info("Saving state: " + name);
  }
  gWasPressed = gIsPressed;

  static bool rWasPressed = false;
  bool rIsPressed = (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS);

  if (rIsPressed && !rWasPressed) {
    resetSimulation();
  }
  rWasPressed = rIsPressed;

  bool leftMousePressed =
      (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

  if (leftMousePressed && !m_leftMouseWasPressed) {
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);

    Ray ray = m_camera->screenToWorldRay(static_cast<float>(m_lastX),
                                         static_cast<float>(m_lastY),
                                         bufferWidth, bufferHeight);

    m_grabbedParticleIndex =
        findClosestParticleToRay(ray, m_solver->getParticles());
    if (m_grabbedParticleIndex != -1) {
      m_isGrabbing = true;
      const Particle& grabbed =
          m_solver->getParticles()[m_grabbedParticleIndex];
      Eigen::Vector3d particlePos = grabbed.getPosition();
      Eigen::Vector3d rayOrigin = ray.getOrigin();
      Eigen::Vector3d rayDir = ray.getDirection();

      m_grabDistance = (particlePos - rayOrigin).dot(rayDir);
    }
  }

  if (m_isGrabbing && leftMousePressed) {
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);

    Ray ray = m_camera->screenToWorldRay(static_cast<float>(m_lastX),
                                         static_cast<float>(m_lastY),
                                         bufferWidth, bufferHeight);

    Eigen::Vector3d newPinPos =
        ray.getOrigin() + m_grabDistance * ray.getDirection().normalized();
    m_solver->addPin(m_grabbedParticleIndex, newPinPos);
  }

  if (!leftMousePressed && m_leftMouseWasPressed) {
    if (m_isGrabbing && m_grabbedParticleIndex != -1)
      m_solver->removePin(m_grabbedParticleIndex);

    m_grabbedParticleIndex = -1;
    m_isGrabbing = false;
  }

  // Hover detection for vertex info overlay
  if (!m_isGrabbing) {
    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);

    Ray hoverRay = m_camera->screenToWorldRay(static_cast<float>(m_lastX),
                                              static_cast<float>(m_lastY),
                                              bufferWidth, bufferHeight);

    m_hoveredParticleIndex =
        findClosestParticleToRay(hoverRay, m_solver->getParticles());
    m_hoveredColliderVertexIndex =
        findClosestColliderVertex(hoverRay, *m_world);
  }

  m_leftMouseWasPressed = leftMousePressed;
}

void Application::update() {
  if (!m_isPaused) m_solver->update(*m_world, 1.0 / 60.0);
}

void Application::render() {
  glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_renderer->render(*m_solver, *m_camera);
  m_renderer->renderColliders(*m_world, *m_camera);

  if (!m_showVertexInfo) return;

  int bufferWidth, bufferHeight;
  glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);
  ImDrawList* drawList = ImGui::GetForegroundDrawList();

  // Draw hovered particle info
  if (m_hoveredParticleIndex != -1) {
    const auto& p =
        m_solver->getParticles()[m_hoveredParticleIndex];
    ImVec2 screenPos =
        worldToScreen(p.getPosition(), bufferWidth, bufferHeight);

    if (screenPos.x >= 0) {
      drawList->AddCircleFilled(screenPos, 6.0f, IM_COL32(255, 60, 60, 255));
      drawList->AddCircle(screenPos, 8.0f, IM_COL32(255, 255, 255, 200), 0,
                          2.0f);

      char label[128];
      snprintf(label, sizeof(label), "Particle %d",
               m_hoveredParticleIndex);
      drawList->AddText(ImVec2(screenPos.x + 14, screenPos.y - 10),
                        IM_COL32(255, 255, 80, 255), label);

      snprintf(label, sizeof(label), "(%.3f, %.3f, %.3f)",
               p.getPosition().x(), p.getPosition().y(),
               p.getPosition().z());
      drawList->AddText(ImVec2(screenPos.x + 14, screenPos.y + 6),
                        IM_COL32(200, 200, 200, 220), label);
    }
  }

  // Draw hovered collider vertex info
  if (m_hoveredColliderVertexIndex != -1 && m_hoveredColliderIndex != -1) {
    const auto& colliders = m_world->getColliders();
    if (m_hoveredColliderIndex < static_cast<int>(colliders.size())) {
      auto* mc = dynamic_cast<const MeshCollider*>(
          colliders[m_hoveredColliderIndex].get());
      if (mc) {
        const auto& wv = mc->getWorldVertices();
        if (m_hoveredColliderVertexIndex < static_cast<int>(wv.size())) {
          const Eigen::Vector3d& vtxPos =
              wv[m_hoveredColliderVertexIndex];
          ImVec2 screenPos =
              worldToScreen(vtxPos, bufferWidth, bufferHeight);

          if (screenPos.x >= 0) {
            drawList->AddCircleFilled(screenPos, 6.0f,
                                     IM_COL32(60, 220, 60, 255));
            drawList->AddCircle(screenPos, 8.0f,
                                IM_COL32(255, 255, 255, 200), 0, 2.0f);

            char label[128];
            snprintf(label, sizeof(label), "Collider Vertex %d",
                     m_hoveredColliderVertexIndex);
            drawList->AddText(
                ImVec2(screenPos.x + 14, screenPos.y - 10),
                IM_COL32(80, 255, 120, 255), label);

            snprintf(label, sizeof(label), "(%.3f, %.3f, %.3f)",
                     vtxPos.x(), vtxPos.y(), vtxPos.z());
            drawList->AddText(
                ImVec2(screenPos.x + 14, screenPos.y + 6),
                IM_COL32(200, 200, 200, 220), label);
          }
        }
      }
    }
  }
}

void Application::shutdown() {
  if (m_window) {
    glfwDestroyWindow(m_window);
  }
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
  Logger::info("Application shutdown complete.");
}

void Application::drawUI() {
  ImGui::Begin("Tissu Control Panel", nullptr,
               ImGuiWindowFlags_AlwaysAutoResize);

  if (ImGui::CollapsingHeader("Configuration IO",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::InputText("Material Path", m_configMaterialPath,
                     sizeof(m_configMaterialPath));

    if (ImGui::Button("Load material")) {
      try {
        ConfigLoader::loadMaterial(m_configMaterialPath,
                                   *(m_cloth->getMaterial()));
        Logger::info("Material loaded from: " +
                     std::string(m_configMaterialPath));
      } catch (const std::exception& e) {
        Logger::error("Failed to load config: " + std::string(e.what()));
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Material Settings")) {
      try {
        ConfigLoader::saveMaterial("exported_material.json",
                                   *(m_cloth->getMaterial()), "exported");
        Logger::info("Material saved to exported_material.json");
      } catch (const std::exception& e) {
        Logger::error("Failed to save config: " + std::string(e.what()));
      }
    }

    ImGui::InputText("Physics Path", m_configPhysicsPath,
                     sizeof(m_configPhysicsPath));

    if (ImGui::Button("Load physics")) {
      try {
        ConfigLoader::loadPhysics(m_configPhysicsPath, *m_solver, *m_world);
        Logger::info("Physics loaded from: " +
                     std::string(m_configMaterialPath));
      } catch (const std::exception& e) {
        Logger::error("Failed to load config: " + std::string(e.what()));
      }
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Physics Settings")) {
      try {
        ConfigLoader::savePhysics("exported_physics.json", *m_solver, *m_world,
                                  "exported");
        Logger::info("Physics saved to exported_physics.json");
      } catch (const std::exception& e) {
        Logger::error("Failed to save config: " + std::string(e.what()));
      }
    }

    if (ImGui::Button("Save Scene")) {
      try {
        SceneExporter::saveScene("exported_scene.json", "exported", *m_solver,
                                 *m_world);
        Logger::info("Scene saved to exported_scene.json");
      } catch (const std::exception& e) {
        Logger::error("Failed to save config: " + std::string(e.what()));
      }
    }
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Shader & Materials", ImGuiTreeNodeFlags_DefaultOpen)) {
    auto& meshes = m_renderer->getClothMeshes();
    for (size_t i = 0; i < meshes.size(); ++i) {
      float col[4] = { meshes[i].color[0], meshes[i].color[1], meshes[i].color[2], meshes[i].color[3] };
      std::string label = "Color: " + meshes[i].name;
      if (ImGui::ColorEdit4(label.c_str(), col)) {
        m_renderer->updateColor(i, col);
      }
    }
    ImGui::Spacing();

    static float ambient = 0.4f;
    if (ImGui::SliderFloat("Ambient", &ambient, 0.0, 1.0))
      m_renderer->updateAmbient(ambient);

    static float diffuse = 0.8f;
    if (ImGui::SliderFloat("Difusse", &diffuse, 0.0, 1.0))
      m_renderer->updateDifusse(diffuse);

    static float sheenAmount = 0.2f;
    if (ImGui::SliderFloat("Sheen Amount", &sheenAmount, 0.0, 1.0))
      m_renderer->updateSheenAmount(sheenAmount);

    static float sheenWidth = 0.4f;
    if (ImGui::SliderFloat("Sheen Width", &sheenWidth, 0.0, 1.0))
      m_renderer->updateSheenWidth(sheenWidth);

    static float anisotropy = 0.8f;
    if (ImGui::SliderFloat("Anisotrophy", &anisotropy, 0.0, 1.0))
      m_renderer->updateAnisotropy(anisotropy);

    static float anisotropyWidth = 0.2f;
    if (ImGui::SliderFloat("Anisotrophy Width", &anisotropyWidth, 0.0, 1.0))
      m_renderer->updateAnisotropyWidth(anisotropyWidth);
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Debug", ImGuiTreeNodeFlags_DefaultOpen)) {
    static bool wireframe = false;

    if (ImGui::Checkbox("Wireframe mode", &wireframe)) {
      if (wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (ImGui::Checkbox("Show Colliders", &m_showColliders)) {
      m_renderer->setShowColliders(m_showColliders);
    }
    ImGui::Checkbox("Show Vertex Info", &m_showVertexInfo);
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Application FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Particles: %d", (int)m_solver->getParticles().size());
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Selection Info", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (m_hoveredParticleIndex != -1) {
      const auto& p = m_solver->getParticles()[m_hoveredParticleIndex];
      ImGui::Text("Particle ID: %d", m_hoveredParticleIndex);
      ImGui::Text("Position: (%.4f, %.4f, %.4f)", p.getPosition().x(),
                  p.getPosition().y(), p.getPosition().z());
      ImGui::Text("Inv Mass: %.6f", p.getInverseMass());
    } else if (m_hoveredColliderVertexIndex != -1) {
      ImGui::Text("Collider #%d  Vertex: %d", m_hoveredColliderIndex,
                  m_hoveredColliderVertexIndex);
      const auto& colliders = m_world->getColliders();
      if (m_hoveredColliderIndex < static_cast<int>(colliders.size())) {
        auto* mc = dynamic_cast<const MeshCollider*>(
            colliders[m_hoveredColliderIndex].get());
        if (mc && m_hoveredColliderVertexIndex <
                      static_cast<int>(mc->getWorldVertices().size())) {
          const auto& vtx =
              mc->getWorldVertices()[m_hoveredColliderVertexIndex];
          ImGui::Text("Position: (%.4f, %.4f, %.4f)", vtx.x(), vtx.y(),
                      vtx.z());
        }
      }
    } else {
      ImGui::TextDisabled("Hover over a vertex to inspect.");
    }
  }

  ImGui::SeparatorText("Playback");
  ImGui::Checkbox("Pause Simulation", &m_isPaused);

  if (ImGui::Button("Reset Scene")) {
    resetSimulation();
  }

  if (ImGui::CollapsingHeader("Global Physics")) {
    static float gY = -9.81f;
    if (ImGui::SliderFloat("Gravity Y", &gY, -20.0f, 2.0f)) {
      Eigen::Vector3d gravityVector(0, gY, 0);
      m_world->setGravity(gravityVector);
      if (m_gravityForce) {
        m_gravityForce->setGravity(gravityVector);
      }
    }

    static int subs = m_solver->getSubsteps();
    if (ImGui::InputInt("Substeps", &subs)) {
      if (subs < 1) subs = 1;
      m_solver->setSubsteps(subs);
    }

    if (ImGui::CollapsingHeader("Wind", ImGuiTreeNodeFlags_DefaultOpen)) {
      static bool windEnabled = false;
      static float windStrength = 1.0f;
      static float windDir[3] = {float(m_world->getWind()[0]),
                                 float(m_world->getWind()[1]),
                                 float(m_world->getWind()[2])};

      ImGui::Checkbox("Enable Wind", &windEnabled);

      ImGui::SliderFloat("Strength", &windStrength, 0.0f, 20.0f);

      ImGui::InputFloat3("Direction", windDir);

      Eigen::Vector3d dir(windDir[0], windDir[1], windDir[2]);

      if (dir.norm() > 1e-6) {
        dir.normalize();
      }

      if (windEnabled) {
        Eigen::Vector3d wind = dir * windStrength;
        m_world->setWind(wind);
        if (m_aeroForce) m_aeroForce->setWind(wind);
      } else {
        m_world->setWind(Eigen::Vector3d::Zero());
        if (m_aeroForce) m_aeroForce->setWind(Eigen::Vector3d::Zero());
      }
    }
  }

  ImGui::End();
}

void Application::resetSimulation() {
  m_solver->softReset();
  syncVisualTopology();
  Logger::info("Simulation reset to initial state.");
}

void Application::syncVisualTopology() {
  if (!m_world || !m_renderer) {
    Logger::warn("Cannot sync topology: World or Renderer not initialized.");
    return;
  }

  m_renderer->clearClothMeshes();

  for (const auto& cloth : m_world->getCloths()) {
    std::vector<unsigned int> triangles;
    for (const auto& tri : cloth->getTriangles()) {
      triangles.push_back(tri.a);
      triangles.push_back(tri.b);
      triangles.push_back(tri.c);
    }
    m_renderer->addClothMesh(cloth->getName(), triangles);
  }
}

ImVec2 Application::worldToScreen(const Eigen::Vector3d& worldPos, int width,
                                  int height) {
  Eigen::Matrix4f viewProj =
      m_camera->getProjectionMatrix() * m_camera->getViewMatrix();
  Eigen::Vector4f pos4(static_cast<float>(worldPos.x()),
                       static_cast<float>(worldPos.y()),
                       static_cast<float>(worldPos.z()), 1.0f);
  Eigen::Vector4f clip = viewProj * pos4;

  if (clip.w() <= 0.0f) return ImVec2(-1, -1);

  Eigen::Vector3f ndc = clip.head<3>() / clip.w();
  float screenX = (ndc.x() + 1.0f) * 0.5f * static_cast<float>(width);
  float screenY = (1.0f - ndc.y()) * 0.5f * static_cast<float>(height);

  return ImVec2(screenX, screenY);
}

int Application::findClosestColliderVertex(const Ray& ray,
                                           const World& world) {
  const auto& colliders = world.getColliders();
  double minDist = 0.1;
  int bestVertex = -1;
  m_hoveredColliderIndex = -1;

  for (int ci = 0; ci < static_cast<int>(colliders.size()); ++ci) {
    auto* mc = dynamic_cast<const MeshCollider*>(colliders[ci].get());
    if (!mc) continue;

    const auto& worldVerts = mc->getWorldVertices();
    Eigen::Vector3d rayOrigin = ray.getOrigin();
    Eigen::Vector3d rayDir = ray.getDirection();

    for (int vi = 0; vi < static_cast<int>(worldVerts.size()); ++vi) {
      Eigen::Vector3d toVert = worldVerts[vi] - rayOrigin;
      double t = toVert.dot(rayDir);
      if (t < 0) continue;
      double distSq = toVert.squaredNorm() - t * t;
      if (distSq < 0) distSq = 0;
      double dist = std::sqrt(distSq);
      if (dist < minDist) {
        minDist = dist;
        bestVertex = vi;
        m_hoveredColliderIndex = ci;
      }
    }
  }

  return bestVertex;
}

int Application::findClosestParticleToRay(
    const Ray& ray, const std::vector<Particle>& particles) {
  if (particles.empty()) return -1;

  Eigen::Vector3d rayOrigin = ray.getOrigin();
  Eigen::Vector3d rayDir = ray.getDirection();

  int closestIndex = -1;
  double minDistance = std::numeric_limits<double>::max();

  const double clickTolerance = 0.1;

  for (int idx = 0; idx < (int)particles.size(); ++idx) {
    Eigen::Vector3d particlePos = particles[idx].getPosition();
    Eigen::Vector3d toParticle = particlePos - rayOrigin;

    double t = toParticle.dot(rayDir);

    if (t < 0) continue;
    double distSq = toParticle.squaredNorm() - (t * t);

    if (distSq < 0) distSq = 0;
    double currentDistance = std::sqrt(distSq);

    if (currentDistance < clickTolerance && currentDistance < minDistance) {
      minDistance = currentDistance;
      closestIndex = idx;
    }
  }

  return closestIndex;
}

}  // namespace Viewer
}  // namespace Tissu