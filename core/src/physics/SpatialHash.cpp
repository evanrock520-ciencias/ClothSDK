// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/SpatialHash.hpp"
#include "physics/Particle.hpp"
#include <cmath>
#include <cstddef>

namespace ClothSDK {

SpatialHash::SpatialHash(int tableSize, double cellSize)
: m_tableSize(tableSize), m_cellSize(cellSize) {}

void SpatialHash::build(const std::vector<Particle>& particles) {
    m_cellStart.assign(m_tableSize + 1, 0); 
    m_particleHashes.resize(particles.size());
    m_particleIndices.resize(particles.size());

    for (size_t i = 0; i < particles.size(); ++i) {
        const Eigen::Vector3d& pos = particles[i].getPosition();
        
        int gx = static_cast<int>(std::floor(pos.x() / m_cellSize));
        int gy = static_cast<int>(std::floor(pos.y() / m_cellSize));
        int gz = static_cast<int>(std::floor(pos.z() / m_cellSize));
        
        int h = hashCoords(gx, gy, gz);
        
        m_particleHashes[i] = h;
        
        m_cellStart[h]++;
    }

    int sum = 0;
    for (int i = 0; i < m_tableSize; ++i) {
        int count = m_cellStart[i];
        m_cellStart[i] = sum;
        sum += count;
    }
    m_cellStart[m_tableSize] = sum;

    std::vector<int> cellOffset = m_cellStart;

    for (size_t i = 0; i < particles.size(); ++i) {
        int hash = m_particleHashes[i];
        int index = cellOffset[hash]++;
        m_particleIndices[index] = i;
    }
}

void SpatialHash::query(const std::vector<Particle>& particles, const Eigen::Vector3d& pos, double radius, std::vector<int>& outNeighbors) const {
    outNeighbors.clear();
    Eigen::Vector3d sphereRadius(radius, radius, radius);
    Eigen::Vector3d pMin = pos - sphereRadius;
    Eigen::Vector3d pMax = pos + sphereRadius;

    int mingx, mingy, mingz;
    int maxgx, maxgy, maxgz;

    posToGrid(pMin, mingx, mingy, mingz);
    posToGrid(pMax, maxgx, maxgy, maxgz);

    for (int x = mingx; x <= maxgx; ++x){
        for (int y = mingy; y <= maxgy; ++y) {
            for (int z = mingz; z <= maxgz; ++z) {
                int hash = hashCoords(x, y, z);
                int start = m_cellStart[hash];
                int end = m_cellStart[hash + 1];
                for (int m = start; m < end; ++m) {
                    int pIndex = m_particleIndices[m];
                    double distance = (particles[pIndex].getPosition() - pos).squaredNorm();
                    if (distance < radius * radius)
                        outNeighbors.push_back(pIndex);
                }
            }
        }
    }
}

}