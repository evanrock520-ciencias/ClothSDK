// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "Renderer.hpp"

#include <glad/gl.h>

#include "Camera.hpp"
#include "Shader.hpp"
#include "physics/Solver.hpp"

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

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return true;
}

void Renderer::render(const Tissu::Solver& solver, const Camera& camera) {
  const auto& particles = solver.getParticles();
  if (particles.empty() || m_clothMeshes.empty()) return;

  const size_t vertexCount = particles.size();

  m_normals.assign(vertexCount, Eigen::Vector3f::Zero());

  for (const auto& mesh : m_clothMeshes) {
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
      const unsigned int ia = mesh.indices[i];
      const unsigned int ib = mesh.indices[i + 1];
      const unsigned int ic = mesh.indices[i + 2];

      if (ia >= vertexCount || ib >= vertexCount || ic >= vertexCount) continue;

      const Eigen::Vector3d& pa = particles[ia].getPosition();
      const Eigen::Vector3d& pb = particles[ib].getPosition();
      const Eigen::Vector3d& pc = particles[ic].getPosition();

      Eigen::Vector3f faceNormal = (pb - pa).cross(pc - pa).cast<float>();

      m_normals[ia] += faceNormal;
      m_normals[ib] += faceNormal;
      m_normals[ic] += faceNormal;
    }
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
  glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size() * sizeof(float),
               m_vertexBuffer.data(), GL_DYNAMIC_DRAW);

  m_shader.bind();

  Eigen::Matrix4f view = camera.getViewMatrix();
  Eigen::Matrix4f proj = camera.getProjectionMatrix();
  Eigen::Vector3f camPos = camera.getPosition();

  m_shader.setMat4("uView", view);
  m_shader.setMat4("uProjection", proj);
  m_shader.setVec3("uViewPos", camPos);

  static const Eigen::Vector3f lightDir =
      Eigen::Vector3f(1.0f, 2.0f, 1.5f).normalized();
  m_shader.setVec3("uLightDir", lightDir);

  glEnable(GL_DEPTH_TEST);

  for (const auto& mesh : m_clothMeshes) {
    if (mesh.indices.empty()) continue;
    m_shader.setVec3("COLOR_FRONT",
                     Eigen::Vector3f(mesh.color[0], mesh.color[1], mesh.color[2]));
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()),
                   GL_UNSIGNED_INT, 0);
  }

  glBindVertexArray(0);
}

void Renderer::cleanup() {
  clearClothMeshes();
  if (m_vao) glDeleteVertexArrays(1, &m_vao);
  if (m_vbo) glDeleteBuffers(1, &m_vbo);
  if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

void Renderer::updateTopology() {
  if (m_clothMeshes.empty()) return;

  m_clothMeshes[0].indices = m_indices;
  glBindVertexArray(m_clothMeshes[0].vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_clothMeshes[0].ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
               m_indices.data(), GL_STATIC_DRAW);
  glBindVertexArray(0);
}

void Renderer::updateColor(float* color) {
  if (!m_clothMeshes.empty()) {
    updateColor(0, color);
  }
}

void Renderer::updateColor(size_t index, float* color) {
  if (index < m_clothMeshes.size()) {
    m_clothMeshes[index].color[0] = color[0];
    m_clothMeshes[index].color[1] = color[1];
    m_clothMeshes[index].color[2] = color[2];
    m_clothMeshes[index].color[3] = color[3];
  }
}

void Renderer::clearClothMeshes() {
  for (auto& mesh : m_clothMeshes) {
    if (mesh.vao) glDeleteVertexArrays(1, &mesh.vao);
    if (mesh.ebo) glDeleteBuffers(1, &mesh.ebo);
  }
  m_clothMeshes.clear();
}

void Renderer::addClothMesh(const std::string& name, const std::vector<unsigned int>& indices) {
  RenderMesh mesh;
  mesh.name = name;
  mesh.indices = indices;

  glGenVertexArrays(1, &mesh.vao);
  glGenBuffers(1, &mesh.ebo);

  glBindVertexArray(mesh.vao);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               indices.data(), GL_STATIC_DRAW);

  constexpr GLsizei stride = 6 * sizeof(float);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                        (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Set a default color
  mesh.color[0] = 0.7f;
  mesh.color[1] = 0.5f;
  mesh.color[2] = 0.5f;
  mesh.color[3] = 1.0f;

  m_clothMeshes.push_back(mesh);
}

void Renderer::updateAmbient(float ambient) {
  m_shader.bind();
  m_shader.setFloat("AMBIENT", ambient);
}

void Renderer::updateDifusse(float difusse) {
  m_shader.bind();
  m_shader.setFloat("DIFFUSE", difusse);
}

void Renderer::updateSheenAmount(float sheenAmount) {
  m_shader.bind();
  m_shader.setFloat("SHEEN_AMOUNT", sheenAmount);
}

void Renderer::updateSheenWidth(float sheenWidth) {
  m_shader.bind();
  m_shader.setFloat("SHEEN_WIDTH", sheenWidth);
}

void Renderer::updateAnisotropy(float anisotropy) {
  m_shader.bind();
  m_shader.setFloat("ANISOTROPY", anisotropy);
}

void Renderer::updateAnisotropyWidth(float anisotropyWidth) {
  m_shader.bind();
  m_shader.setFloat("ANISOTROPY_WIDTH", anisotropyWidth);
}

}  // namespace Viewer
}  // namespace Tissu