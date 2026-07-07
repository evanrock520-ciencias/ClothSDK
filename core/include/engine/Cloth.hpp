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

#include <cmath>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "math/Types.hpp"
#include "physics/AerodynamicForce.hpp"

namespace Tissu {

/**
 * @enum ClothTopology
 * @brief Describes the construction method of a cloth object.
 */
enum class ClothTopology {
  Grid,  ///< Procedurally generated uniform grid via @ref ClothMesh::initGrid.
  Mesh   ///< Topology loaded from a Wavefront OBJ file via @ref
         ///< ClothMesh::buildFromMesh.
};

/**
 * @class Cloth
 * @brief Represents a simulated garment within the physics world.
 *
 * A Cloth holds the topology, material properties, and particle references
 * that define a single piece of fabric. It does not own the particles —
 * those live in the @ref Solver. The Cloth acts as a view over the solver's
 * particle buffer, identifying which particles and constraints belong to it.
 *
 * Cloth objects are registered into a @ref World and updated each frame
 * by the @ref Solver.
 */
class Cloth {
 public:
  Cloth(const std::string& name, std::shared_ptr<ClothMaterial> material);

  /**
   * @brief Returns the particle's ID using the rows and cols as parameters to
   * search in the indice's buffer.
   *
   * @param r The row of the particle.
   * @param c The column of the particle.
   * @return int The particle's ID.
   */
  inline int getParticleID(int r, int c) const {
    int localIndex = r * m_gridCols + c;
    return m_particleIndices[localIndex];
  }

  /**
   * @brief Clears the indices, triangles, and aerofaces buffers.
   *
   */
  void clear();

  inline void addAeroFace(int a, int b, int c) { m_faces.push_back({a, b, c}); }
  void addParticleId(int id);
  void addTriangle(const Triangle& tri);
  void addVisualEdge(unsigned int idA, unsigned int idB);

  void setName(const std::string& name);
  void setMaterial(std::shared_ptr<ClothMaterial> material);
  void setGridDimensions(int rows, int cols);
  void setTopology(ClothTopology topology);
  inline void setRestVolume(double restVolume) { m_restVolume = restVolume; }
  inline void setSpacing(double spacing) { m_spacing = spacing; }
  inline void setPin(const Pin& pin) { m_pin = pin; }
  inline void setMeshPath(const std::string& meshPath) {
    m_meshPath = meshPath;
  }
  inline void setTranslation(const Eigen::Vector3d& translation) {
    m_translation = translation;
  }
  inline void setRotation(const Eigen::Quaterniond& rotation) {
    m_rotation = rotation;
  }

  inline const std::string& getName() const { return m_name; }
  inline const ClothTopology getTopology() const { return m_topology; }
  inline std::shared_ptr<ClothMaterial> getMaterial() const {
    return m_material;
  }
  inline const std::vector<int>& getParticleIndices() const {
    return m_particleIndices;
  }
  inline const std::vector<Triangle>& getTriangles() const {
    return m_triangles;
  }
  inline const std::vector<unsigned int>& getVisualEdges() const {
    return m_visualEdges;
  }
  inline const std::vector<AeroFace>& getAeroFaces() const { return m_faces; }
  inline const int getRows() const { return m_gridRows; }
  inline const int getCols() const { return m_gridCols; }
  inline const double getRestVolume() const { return m_restVolume; }
  inline const double getSpacing() const { return m_spacing; }
  inline const Pin& getPin() const { return m_pin; }
  inline const std::string& getMeshPath() const { return m_meshPath; }
  inline const Eigen::Vector3d& getTranslation() const { return m_translation; }
  inline const Eigen::Quaterniond& getRotation() const { return m_rotation; }

  bool isGrid() const { return m_topology == ClothTopology::Grid; }
  bool isMesh() const { return m_topology == ClothTopology::Mesh; }
  bool isClosed() const;

 private:
  std::string m_name;
  ClothTopology m_topology;
  std::string m_meshPath;
  std::map<Edge, int> m_edgeFaceCount;
  std::shared_ptr<ClothMaterial> m_material;
  std::vector<int> m_particleIndices;
  std::vector<Triangle> m_triangles;
  std::vector<unsigned int> m_visualEdges;
  std::vector<AeroFace> m_faces;
  int m_gridRows;
  int m_gridCols;
  double m_restVolume = INFINITY;
  double m_spacing;
  Eigen::Vector3d m_translation = Eigen::Vector3d::Zero();
  Eigen::Quaterniond m_rotation = Eigen::Quaterniond::Identity();
  Pin m_pin;
};

}  // namespace Tissu