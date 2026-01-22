// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <memory>
#include <Eigen/Dense>

#include "Application.hpp"
#include "engine/ClothMesh.hpp"
#include "utils/Logger.hpp"
#include "physics/Solver.hpp"
#include "physics/Particle.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"
#include "io/ConfigLoader.hpp" 

extern IMGUI_IMPL_API void ImGui_ImplGlfw_CursorPosCallback(GLFWwindow* window, double x, double y);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
extern IMGUI_IMPL_API void ImGui_ImplGlfw_CharCallback(GLFWwindow* window, unsigned int c);

namespace ClothSDK {
namespace Viewer {

Application::Application() 
    : m_window(nullptr), 
    m_solver(nullptr), 
    m_renderer(nullptr), 
    m_camera(nullptr), 
    m_deltaTime(0.0), 
    m_lastFrame(0.0),
    m_isPaused(false)
{
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

bool Application::init(int width, int height, const std::string& title, const std::string& shaderPath) {
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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::error("Failed to initialize GLAD");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    float fontSize = 24.0f; 
    float scale = 2.0f; 
    io.Fonts->AddFontFromFileTTF("/usr/share/fonts/liberation-sans-fonts/LiberationSans-Regular.ttf", fontSize);
    ImGui::GetStyle().ScaleAllSizes(scale);
    io.FontGlobalScale = 1.0f; 

    ImGui_ImplGlfw_InitForOpenGL(m_window, false);
    ImGui_ImplOpenGL3_Init("#version 330");

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
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

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods) {
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
        ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

        if (ImGui::GetIO().WantCaptureMouse) return;
        
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->m_camera->handleZoom(static_cast<float>(yoffset));
    });

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
        
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    });

    glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int c) {
        ImGui_ImplGlfw_CharCallback(window, c);
    });

    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app->m_camera && height > 0) {
            app->m_camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
        }
    });

    m_world = std::make_shared<World>();
    if (!m_solver) m_solver = std::make_shared<Solver>();
    if (!m_mesh)   m_mesh   = std::make_shared<ClothMesh>();

    m_renderer = std::make_unique<Renderer>();
    m_renderer->setShaderPath(shaderPath);

    if (!m_renderer->init()) {
        Logger::error("Failed to initialize Renderer with shader path: " + shaderPath);
        return false;
    }

    int bufferWidth, bufferHeight;
    glfwGetFramebufferSize(m_window, &bufferWidth, &bufferHeight);

    m_camera = std::make_unique<Camera>(Eigen::Vector3f(0, 5, 10), Eigen::Vector3f(0, 2, 0));
    
    if (bufferHeight > 0) {
        m_camera->setAspectRatio(static_cast<float>(bufferWidth) / static_cast<float>(bufferHeight));
        glViewport(0, 0, bufferWidth, bufferHeight);
    }
    
    Logger::info("Window Size: " + std::to_string(width) + "x" + std::to_string(height));
    Logger::info("Framebuffer Size: " + std::to_string(bufferWidth) + "x" + std::to_string(bufferHeight));
    
    Logger::info("ClothSDK Viewer initialized successfully: OpenGL 3.3 Core Profile");
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

    if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
        resetSimulation();
    }
}

void Application::update() {
    if (!m_isPaused)
        m_solver->update(*m_world, m_deltaTime);
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
    ImGui::Begin("ClothSDK Control Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::CollapsingHeader("Configuration IO", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::InputText("Config Path", m_configPathBuffer, sizeof(m_configPathBuffer));

        if (ImGui::Button("Load JSON Config")) {
            if (ConfigLoader::load(m_configPathBuffer, *m_solver, *m_world, *(m_cloth->getMaterial()))) {
                Logger::info("Configuration loaded successfully from: " + std::string(m_configPathBuffer));
            } else {
                Logger::error("Failed to load config: " + std::string(m_configPathBuffer));
            }
        }
        
        ImGui::SameLine();

        if (ImGui::Button("Save Current Settings")) {
            if (ConfigLoader::save("exported_config.json", *m_solver, *m_world, *(m_cloth->getMaterial()))) {
                Logger::info("Settings saved to exported_config.json");
            }
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
            m_world->setGravity(Eigen::Vector3d(0, gY, 0));
        }

        static int subs = m_solver->getSubsteps();
        if (ImGui::InputInt("Substeps", &subs)) {
            if (subs < 1) subs = 1;
            m_solver->setSubsteps(subs);
        }

        if (ImGui::CollapsingHeader("Wind", ImGuiTreeNodeFlags_DefaultOpen)) {
            static bool windEnabled = true;
            static float windStrength = 5.0f;
            static float windDir[3] = {1.0f, 0.0f, 0.0f};

            ImGui::Checkbox("Enable Wind", &windEnabled);

            ImGui::SliderFloat("Strength", &windStrength, 0.0f, 20.0f);

            ImGui::InputFloat3("Direction", windDir);

            Eigen::Vector3d dir(windDir[0], windDir[1], windDir[2]);

            if (dir.norm() > 1e-6) {
                dir.normalize();
            }

            if (windEnabled) {
                m_world->setWind(dir * windStrength);
            } else {
                m_world->setWind(Eigen::Vector3d::Zero());
            }
        }
    }

    ImGui::End();
}

void Application::resetSimulation() {
    m_solver->clear();
    
    int rows = (m_initRows > 0) ? m_initRows : 20;
    int cols = (m_initCols > 0) ? m_initCols : 20;
    float spacing = (m_initSpacing > 0.0f) ? m_initSpacing : 0.1f;

    m_mesh->initGrid(rows, cols, spacing, *m_cloth, *m_solver);
    
    syncVisualTopology();
    
    Logger::info("Simulation Reset (Grid: " + std::to_string(rows) + "x" + std::to_string(cols) + ")");
}

void Application::syncVisualTopology() {
    if (!m_mesh || !m_renderer) {
        Logger::warn("Cannot sync topology: Mesh or Renderer not initialized.");
        return;
    }

    const std::vector<unsigned int>& edges = m_cloth->getVisualEdges();
    m_renderer->setIndices(edges);
    m_renderer->updateTopology();
}

} 
}