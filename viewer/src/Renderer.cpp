#include <glad/glad.h>
#include "Renderer.hpp"
#include "physics/Solver.hpp"
#include "physics/Particle.hpp"
#include "Camera.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <sstream>

namespace ClothSDK {
namespace Viewer {

Renderer::Renderer() {}
Renderer::~Renderer() { cleanup(); }

bool Renderer::init() {
    m_shaderProgram = compileShaders(m_shaderPath + "cloth.vert", m_shaderPath + "cloth.frag");
    if (m_shaderProgram == 0) return false;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                m_indices.size() * sizeof(unsigned int), 
                m_indices.data(), 
                GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void Renderer::render(const ClothSDK::Solver& solver, const Camera& camera) {
    const auto& particles = solver.getParticles();
    if (particles.empty()) return;

    m_vertexBuffer.clear();
    m_vertexBuffer.reserve(particles.size() * 3);
    for (const auto& p : particles) {
        Eigen::Vector3d pos = p.getPosition();
        m_vertexBuffer.push_back(static_cast<float>(pos.x()));
        m_vertexBuffer.push_back(static_cast<float>(pos.y()));
        m_vertexBuffer.push_back(static_cast<float>(pos.z()));
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size() * sizeof(float), m_vertexBuffer.data(), GL_DYNAMIC_DRAW);

    glUseProgram(m_shaderProgram);

    Eigen::Matrix4f view = camera.getViewMatrix();
    Eigen::Matrix4f proj = camera.getProjectionMatrix();

    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"), 1, GL_FALSE, view.data());
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"), 1, GL_FALSE, proj.data());

    glBindVertexArray(m_vao);
    glDrawElements(GL_LINES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    glPointSize(5.0f);
    glDrawArrays(GL_POINTS, 0, (GLsizei)particles.size());
    
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}


unsigned int Renderer::compileShaders(const std::string& vPath, const std::string& fPath) {
    std::string vCode = loadFile(vPath);
    std::string fCode = loadFile(fPath);
    if (vCode.empty() || fCode.empty()) return 0;

    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    int success;
    char infoLog[512];

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        Logger::error("Vertex Shader Compilation Failed: " + std::string(infoLog));
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        Logger::error("Fragment Shader Compilation Failed: " + std::string(infoLog));
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        Logger::error("Shader Linking Failed: " + std::string(infoLog));
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

std::string Renderer::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::error("Could not open shader file: " + path);
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

} 
} 