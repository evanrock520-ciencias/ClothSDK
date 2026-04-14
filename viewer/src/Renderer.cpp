// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include <glad/glad.h>
#include "Renderer.hpp"
#include "Shader.hpp"
#include "physics/Solver.hpp"
#include "Camera.hpp"

namespace Tissu {
namespace Viewer {

Renderer::Renderer() : m_shader("", "") {}
Renderer::~Renderer() { cleanup(); }

bool Renderer::init() {
    std::string vertPath = m_shaderPath + "cloth.vert";
    std::string fragPath = m_shaderPath + "cloth.frag";
    
    m_shader = Shader(vertPath, fragPath);
    if (!m_shader.init()) {
        return false;
    }

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

    m_shader.bind();

    Eigen::Matrix4f view = camera.getViewMatrix();
    Eigen::Matrix4f proj = camera.getProjectionMatrix();
    Eigen::Vector3f camPos = camera.getPosition();

    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", proj);
    m_shader.setVec3("uViewPos", camPos);

    static const Eigen::Vector3f lightDir = Eigen::Vector3f(1.0f, 2.0f, 1.5f).normalized();
    m_shader.setVec3("uLightDir", lightDir);

    glBindVertexArray(m_vao);
    glEnable(GL_DEPTH_TEST);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
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
}

void Renderer::updateColor(float* color) {
    m_shader.bind();
    m_shader.setVec3("COLOR_FRONT", Eigen::Vector3f(color[0], color[1], color[2]));
}

} 
} 