#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "Application.hpp"
#include "engine/ClothMesh.hpp"
#include "utils/Logger.hpp"
#include "physics/Solver.hpp"
#include "physics/Particle.hpp"
#include "Renderer.hpp"
#include "Camera.hpp"

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

}

Application::~Application() = default;

bool Application::init(int width, int height, const std::string& title, const std::string& shaderPath) {
    if(!glfwInit()) return false;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
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

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
            app->m_camera->handleMouse(xoffset, yoffset);
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->m_camera->handleZoom(static_cast<float>(yoffset));
    });

    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        
        auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app->m_camera) {
            app->m_camera->setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
        }
    });
    
    if (!m_solver) {
        m_solver = std::make_shared<Solver>();
    }

    if (!m_mesh) {
        m_mesh = std::make_shared<ClothMesh>();
    }

    m_renderer = std::make_unique<Renderer>();
    m_renderer->setShaderPath(shaderPath);

    if (!m_renderer->init()) {
        Logger::error("Failed to initialize Renderer with path: " + shaderPath);
        return false;
    }

    m_camera = std::make_unique<Camera>(Eigen::Vector3f(0, 5, 10), Eigen::Vector3f(1, 1, 0));

    m_mesh->initGrid(20, 20, 0.1, *m_solver); 

    m_renderer->setIndices(m_mesh->getVisualEdges());

    if (!m_renderer->init()) {
        Logger::error("Failed to initialize Renderer");
        return false;
    }

    m_camera->setAspectRatio((float)width / (float)height);
    for(int i = 0; i < 20; ++i) 
        m_solver->setParticleInverseMass(m_mesh->getParticleID(19, i), 0.0);

    glViewport(0, 0, width, height);
    
    Logger::info("Renderer initialized: OpenGL 3.3 Core");
    return true;
}

void Application::run() {
    m_lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(m_window)) {
        double currentFrame = glfwGetTime();
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        if (m_deltaTime > 0.05) m_deltaTime = 0.05;

        processInput();
        update();
        render();

        glfwSwapBuffers(m_window);
        glfwPollEvents();
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
    m_solver->clear(); 
    m_mesh->initGrid(20, 20, 0.1, *m_solver); 
    m_renderer->setIndices(m_mesh->getVisualEdges()); 
    for(int i = 0; i < 20; ++i) 
        m_solver->setParticleInverseMass(m_mesh->getParticleID(19, i), 0.0);
    
    Logger::info("Simulation Reset");
}
}

void Application::update() {
    if (!m_isPaused)
        m_solver->update(m_deltaTime);
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
    glfwTerminate();
    Logger::info("Application shutdown complete.");
}

} 
}