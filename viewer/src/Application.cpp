// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "Application.hpp"

#include <GLFW/glfw3.h>
#include <glad/gl.h>
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
#ifdef _WIN32
  io.Fonts->AddFontDefault();
#else
  io.Fonts->AddFontFromFileTTF(
      "/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf",
      fontSize);
#endif
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

  m_leftMouseWasPressed = leftMousePressed;
}

void Application::update() {
  if (!m_isPaused) m_solver->update(*m_world, 1.0 / 60.0);
}

void Application::render() {
  glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_renderer->render(*m_solver, *m_camera);
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

  if (ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::ColorPicker4("Color", m_color)) {
      m_renderer->updateColor(m_color);
    }

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
  }

  ImGui::Separator();

  if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Application FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Particles: %d", (int)m_solver->getParticles().size());
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
  if (!m_mesh || !m_renderer) {
    Logger::warn("Cannot sync topology: Mesh or Renderer not initialized.");
    return;
  }

  std::vector<unsigned int> triangles;
  for (const auto& tri : m_cloth->getTriangles()) {
    triangles.push_back(tri.a);
    triangles.push_back(tri.b);
    triangles.push_back(tri.c);
  }

  m_renderer->setIndices(triangles);
  m_renderer->updateTopology();
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