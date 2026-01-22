#pragma once

#include "math/Types.hpp"
#include "physics/AerodynamicForce.hpp"
#include <memory>
#include <string>
#include <vector>

namespace ClothSDK {

class Cloth {
public:
    Cloth(const std::string& name, std::shared_ptr<ClothMaterial> material);

    void setName(const std::string& name);
    void setMaterial(std::shared_ptr<ClothMaterial> material);
    void setGridDimensions(int rows, int cols);

    inline const std::string& getName() const { return m_name; }
    inline std::shared_ptr<ClothMaterial> getMaterial() const { return m_material; }
    inline const std::vector<int>& getParticleIndices() const { return m_particleIndices; }
    inline const std::vector<Triangle>& getTriangles() const { return m_triangles; }
    inline const std::vector<unsigned int>& getVisualEdges() const { return m_visualEdges; }
    inline void addAeroFace(int a, int b, int c) { m_faces.push_back({a, b, c}); }
    inline const std::vector<AeroFace>& getAeroFaces() const { return m_faces; }
    inline int getParticleID(int r, int c) const { 
        int localIndex = r * m_gridCols + c;
        return m_particleIndices[localIndex];
    }

    void addParticleId(int id);
    void addTriangle(const Triangle& tri);
    void addVisualEdge(unsigned int idA, unsigned int idB);

    void clear();

private:
    std::string m_name;
    std::shared_ptr<ClothMaterial> m_material;
    std::vector<int> m_particleIndices;
    std::vector<Triangle> m_triangles;
    std::vector<unsigned int> m_visualEdges;
    std::vector<AeroFace> m_faces;
    int m_gridRows;
    int m_gridCols;
};

}