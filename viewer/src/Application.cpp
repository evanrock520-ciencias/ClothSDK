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
    m_lastFrame(0.0) 
{

}

Application::~Application() = default;

bool Application::init(int width, int height, const std::string& title) {
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
    glfwSwapInterval(1); 

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        Logger::error("Failed to initialize GLAD");
        return false; 
    }
    m_solver = std::make_unique<Solver>();
    m_renderer = std::make_unique<Renderer>();
    m_camera = std::make_unique<Camera>();

    if (!m_renderer->init()) {
        Logger::error("Failed to initialize Renderer (Shaders/Buffers)");
        return false;
    }

    m_camera->setAspectRatio((float)width / (float)height);

    ClothMesh mesh; 
    mesh.initGrid(20, 20, 0.1, *m_solver); 

    for(int i = 0; i < 20; ++i) 
        m_solver->setParticleInverseMass(mesh.getParticleID(19, i), 0.0);

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
}

void Application::update() {
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