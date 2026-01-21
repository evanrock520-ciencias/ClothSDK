#pragma once

#include "math/Types.hpp"
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
    int m_gridRows;
    int m_gridCols;
};

}