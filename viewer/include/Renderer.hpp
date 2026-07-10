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
class World;
namespace Viewer {
class Camera;

struct RenderMesh {
    unsigned int vao = 0;
    unsigned int ebo = 0;
    std::vector<unsigned int> indices;
    std::string name;
    float color[4] = {0.99f, 0.96f, 0.72f, 1.0f};
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();

    void render(const Tissu::Solver& solver, const Camera& camera);
    void renderColliders(const Tissu::World& world, const Camera& camera);
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
    void addClothMesh(const std::string& name,
                      const std::vector<unsigned int>& indices);
    inline const std::vector<RenderMesh>& getClothMeshes() const {
        return m_clothMeshes;
    }
    inline std::vector<RenderMesh>& getClothMeshes() { return m_clothMeshes; }

    inline void setIndices(const std::vector<unsigned int>& indices) {
        m_indices = indices;
        clearClothMeshes();
        addClothMesh("DefaultCloth", indices);
    }
    inline void setShaderPath(const std::string& path) { m_shaderPath = path; }
    inline void setShowColliders(bool v) { m_showColliders = v; }
    inline bool getShowColliders() const { return m_showColliders; }

private:
    Shader m_shader;
    Shader m_colliderShader;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_ebo = 0;

    std::vector<float> m_vertexBuffer;
    std::vector<unsigned int> m_indices;
    std::vector<Eigen::Vector3f> m_normals;
    std::string m_shaderPath = "../viewer/shaders/";
    bool m_showColliders = true;

    // Debug buffers for collider wireframe rendering
    unsigned int m_debugVao = 0;
    unsigned int m_debugVbo = 0;
    unsigned int m_debugEbo = 0;

    // Geometry builders for analytic colliders
    void buildSphereLines(const Eigen::Vector3d& center, double radius,
                          std::vector<float>& verts,
                          std::vector<unsigned int>& indices);
    void buildCapsuleLines(const Eigen::Vector3d& start,
                           const Eigen::Vector3d& end, double radius,
                           std::vector<float>& verts,
                           std::vector<unsigned int>& indices);
    void buildPlaneLines(const Eigen::Vector3d& origin,
                         const Eigen::Vector3d& normal,
                         std::vector<float>& verts,
                         std::vector<unsigned int>& indices);

    std::vector<RenderMesh> m_clothMeshes;
};
} // namespace Viewer
} // namespace Tissu