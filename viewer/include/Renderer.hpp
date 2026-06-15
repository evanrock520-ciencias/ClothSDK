/*
 * Copyright 2026 Evan M.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <Eigen/Dense>
#include <string>
#include <vector>

#include "Shader.hpp"

namespace Tissu {
class Solver;
namespace Viewer {
class Camera;

struct RenderMesh {
  unsigned int vao = 0;
  unsigned int ebo = 0;
  std::vector<unsigned int> indices;
  std::string name;
  float color[4] = {0.7f, 0.5f, 0.5f, 1.0f};
};

class Renderer {
 public:
  Renderer();
  ~Renderer();

  bool init();

  void render(const Tissu::Solver& solver, const Camera& camera);
  void cleanup();
  void updateTopology();

  void updateColor(float* color);
  void updateColor(size_t index, float* color);
  void updateAmbient(float ambient);
  void updateDifusse(float difusse);
  void updateSheenAmount(float sheenAmount);
  void updateSheenWidth(float sheenWidth);
  void updateAnisotropy(float anisotropy);
  void updateAnisotropyWidth(float anisotropyWidth);

  void clearClothMeshes();
  void addClothMesh(const std::string& name, const std::vector<unsigned int>& indices);
  inline const std::vector<RenderMesh>& getClothMeshes() const { return m_clothMeshes; }
  inline std::vector<RenderMesh>& getClothMeshes() { return m_clothMeshes; }

  inline void setIndices(const std::vector<unsigned int>& indices) {
    m_indices = indices;
    clearClothMeshes();
    addClothMesh("DefaultCloth", indices);
  }
  inline void setShaderPath(const std::string& path) { m_shaderPath = path; }

 private:
  Shader m_shader;
  unsigned int m_vao = 0;
  unsigned int m_vbo = 0;
  unsigned int m_ebo = 0;

  std::vector<float> m_vertexBuffer;
  std::vector<unsigned int> m_indices;
  std::vector<Eigen::Vector3f> m_normals;
  std::string m_shaderPath = "../viewer/shaders/";

  std::vector<RenderMesh> m_clothMeshes;
};
}  // namespace Viewer
}  // namespace Tissu