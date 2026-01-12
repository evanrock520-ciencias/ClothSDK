#pragma once

#include <memory>
#include <string>

struct GLFWwindow;

namespace ClothSDK {

class Solver;
class ClothMesh;

namespace Viewer {

class Renderer;
class Camera;

class Application {
public:
    Application();
    ~Application();
    
    bool init(int width, int height, const std::string& title, const std::string& shaderPath);
    void run();
    void shutdown();

    inline void setSolver(std::shared_ptr<Solver> solver) { m_solver = solver; }
    inline void setMesh(std::shared_ptr<ClothMesh> mesh) { m_mesh = mesh; }
    inline Renderer& getRenderer() { return *m_renderer; }

private:
    void processInput();
    void update();
    void render();

    GLFWwindow* m_window;
    std::shared_ptr<Solver> m_solver;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Camera> m_camera;
    std::shared_ptr<ClothMesh> m_mesh; 
    double m_deltaTime;
    double m_lastFrame;

    double m_lastX = 0.0;
    double m_lastY = 0.0;
    bool m_firstMouse = true;
};

}
}