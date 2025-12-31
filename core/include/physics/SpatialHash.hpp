#pragma once

#include <vector>
#include <Eigen/Dense>

namespace ClothSDK {

class Particle;

class SpatialHash {
public:
    SpatialHash(int tableSize, double cellSize);
    void build(const std::vector<Particle>& particles);
    void query(const Eigen::Vector3d& pos, double radius, std::vector<int>& outNeighbors);

    void setCellSize(double h) { m_cellSize = h; }
    double getCellSize() const { return m_cellSize; }
private:
    inline int hashCoords(int x, int y, int z) const {
        return (std::abs(x * 73856093 ^ y * 19349663 ^ z * 83492791)) % m_tableSize;
    }

    inline void posToGrid(const Eigen::Vector3d& pos, int& gx, int& gy, int& gz) const {
        gx = static_cast<int>(std::floor(pos.x() / m_cellSize));
        gy = static_cast<int>(std::floor(pos.y() / m_cellSize));
        gz = static_cast<int>(std::floor(pos.z() / m_cellSize));
    }

    int m_tableSize;
    double m_cellSize;
    std::vector<int> m_cellStart;
    std::vector<int> m_particleIndices;
    std::vector<int> m_particleHashes;
};

}