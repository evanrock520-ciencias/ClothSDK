#pragma once

#include <memory>
#include <string>

struct GLFWwindow;

namespace ClothSDK {

class Solver;

namespace Viewer {

class Renderer;
class Camera;

class Application {
public:
    Application();
    ~Application();
    
    bool init(int width, int height, const std::string& title);
    void run();
    void shutdown();

private:
    void processInput();
    void update();
    void render();

    GLFWwindow* m_window;
    std::unique_ptr<Solver> m_solver;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Camera> m_camera;
    double m_deltaTime;
    double m_lastFrame;

    double m_lastX = 0.0;
    double m_lastY = 0.0;
    bool m_firstMouse = true;
};

}
}