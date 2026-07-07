// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "engine/Cloth.hpp"

#include <memory>

namespace Tissu {

Cloth::Cloth(const std::string& name, std::shared_ptr<ClothMaterial> material)
    : m_name(name),
      m_material(material),
      m_gridCols(0),
      m_gridRows(0),
      m_topology(ClothTopology::Mesh) {}

void Cloth::setName(const std::string& name) { m_name = name; }
void Cloth::setMaterial(std::shared_ptr<ClothMaterial> material) {
  m_material = material;
}
void Cloth::setGridDimensions(int rows, int cols) {
  m_gridCols = cols;
  m_gridRows = rows;
}
void Cloth::setTopology(ClothTopology topology) { m_topology = topology; }

void Cloth::addParticleId(int id) { m_particleIndices.push_back(id); }
void Cloth::addTriangle(const Triangle& tri) { m_triangles.push_back(tri); }
void Cloth::addVisualEdge(unsigned int idA, unsigned int idB) {
  m_visualEdges.push_back(idA);
  m_visualEdges.push_back(idB);
}

bool Cloth::isClosed() const {
  std::map<Edge, int> edgeCount;
  for (const auto& tri : m_triangles) {
    edgeCount[{tri.a, tri.b}]++;
    edgeCount[{tri.b, tri.c}]++;
    edgeCount[{tri.c, tri.a}]++;
  }
  for (const auto& [edge, count] : edgeCount)
    if (count != 2) return false;

  return true;
}

void Cloth::clear() {
  m_particleIndices.clear();
  m_triangles.clear();
  m_visualEdges.clear();
  m_faces.clear();
  m_edgeFaceCount.clear();
}
}  // namespace Tissu