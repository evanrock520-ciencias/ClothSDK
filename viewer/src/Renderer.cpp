// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <glad/glad.h>
#include "Renderer.hpp"
#include "physics/Solver.hpp"
#include "physics/Particle.hpp"
#include "Camera.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <sstream>
#include <string>

namespace Tissu {
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

    constexpr GLsizei stride = 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void Renderer::render(const Tissu::Solver& solver, const Camera& camera) {
    const auto& particles = solver.getParticles();
    if (particles.empty() || m_indices.empty()) return;

    const size_t vertexCount = particles.size();

    m_normals.assign(vertexCount, Eigen::Vector3f::Zero());

    for (size_t i = 0; i + 2 < m_indices.size(); i += 3) {
        const unsigned int ia = m_indices[i];
        const unsigned int ib = m_indices[i + 1];
        const unsigned int ic = m_indices[i + 2];

        if (ia >= vertexCount || ib >= vertexCount || ic >= vertexCount)
            continue;

        const Eigen::Vector3d& pa = particles[ia].getPosition();
        const Eigen::Vector3d& pb = particles[ib].getPosition();
        const Eigen::Vector3d& pc = particles[ic].getPosition();

        Eigen::Vector3f faceNormal = (pb - pa).cross(pc - pa).cast<float>();

        m_normals[ia] += faceNormal;
        m_normals[ib] += faceNormal;
        m_normals[ic] += faceNormal;
    }

    m_vertexBuffer.clear();
    m_vertexBuffer.reserve(vertexCount * 6);

    for (size_t i = 0; i < vertexCount; ++i) {
        const Eigen::Vector3d& pos = particles[i].getPosition();
        m_vertexBuffer.push_back(static_cast<float>(pos.x()));
        m_vertexBuffer.push_back(static_cast<float>(pos.y()));
        m_vertexBuffer.push_back(static_cast<float>(pos.z()));

        Eigen::Vector3f n = m_normals[i];
        float len = n.norm();
        if (len > 1e-6f) n /= len;

        m_vertexBuffer.push_back(n.x());
        m_vertexBuffer.push_back(n.y());
        m_vertexBuffer.push_back(n.z());
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m_vertexBuffer.size() * sizeof(float),
                 m_vertexBuffer.data(),
                 GL_DYNAMIC_DRAW);

    glUseProgram(m_shaderProgram);

    Eigen::Matrix4f view = camera.getViewMatrix();
    Eigen::Matrix4f proj = camera.getProjectionMatrix();
    Eigen::Vector3f camPos = camera.getPosition();

    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uView"),       1, GL_FALSE, view.data());
    glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "uProjection"), 1, GL_FALSE, proj.data());
    glUniform3fv      (glGetUniformLocation(m_shaderProgram, "uViewPos"),    1, camPos.data());

    static const Eigen::Vector3f lightDir = Eigen::Vector3f(1.0f, 2.0f, 1.5f).normalized();
    glUniform3fv(glGetUniformLocation(m_shaderProgram, "uLightDir"), 1, lightDir.data());

    glBindVertexArray(m_vao);
    glEnable(GL_DEPTH_TEST);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

void Renderer::updateTopology() {
    if (m_vao == 0 || m_ebo == 0) return;

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_indices.size() * sizeof(unsigned int),
                 m_indices.data(),
                 GL_STATIC_DRAW);
    glBindVertexArray(0);

    Logger::info("GPU Topology updated: " + std::to_string(m_indices.size() / 3) + " triangles.");
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

void Renderer::updateColor(float* color) {
    glUseProgram(m_shaderProgram);
    GLint uniformColor = glGetUniformLocation(m_shaderProgram, "COLOR_FRONT");
    glUniform3f(uniformColor, color[0], color[1], color[2]);
}

} 
} 