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

class Renderer {
 public:
  Renderer();
  ~Renderer();

  bool init();

  void render(const Tissu::Solver& solver, const Camera& camera);
  void cleanup();
  void updateTopology();

  void updateColor(float* color);
  void updateAmbient(float ambient);
  void updateDifusse(float difusse);
  void updateSheenAmount(float sheenAmount);
  void updateSheenWidth(float sheenWidth);
  void updateAnisotropy(float anisotropy);
  void updateAnisotropyWidth(float anisotropyWidth);

  inline void setIndices(const std::vector<unsigned int>& indices) {
    m_indices = indices;
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
};
}  // namespace Viewer
}  // namespace Tissu