#pragma once

#include "Eigen/Dense"

namespace ClothSDK {

class Solver;

namespace Viewer {

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void render(const ClothSDK::Solver& solver);
    void cleanup();

private:
    unsigned int compileShaders(const char* vertexSrc, const char* fragmentSrc);

    unsigned int m_shaderProgram;
    unsigned int m_vao; 
    unsigned int m_vbo; 
    unsigned int m_ebo; 

    std::vector<float> m_vertexBuffer;
};

}
}