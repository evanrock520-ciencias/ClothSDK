// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "data-structures/SpatialHash.hpp"

#include <cmath>
#include <cstddef>
#include <tracy/Tracy.hpp>

#include "physics/Particle.hpp"

namespace Tissu {

SpatialHash::SpatialHash(int tableSize, double cellSize)
    : m_cellSize(cellSize),
      m_invCellSize((cellSize > 1e-12) ? (1.0 / cellSize) : 1.0) {
    int p2 = 1;
    while (p2 < tableSize) {
        p2 <<= 1;
    }
    m_tableSize = p2;
    m_tableMask = static_cast<unsigned int>(p2 - 1);
}

void SpatialHash::build(const std::vector<Particle>& particles) {
    ZoneScopedN("SpatialHash Build");
    m_cellStart.assign(m_tableSize + 1, 0);
    m_particleHashes.resize(particles.size());
    m_particleIndices.resize(particles.size());
    m_occupiedCells.clear();

    // Per-particle grid coordinates (kept for occupied-cell detection).
    struct GridCoord {
        int gx, gy, gz;
    };
    std::vector<GridCoord> gridCoords(particles.size());

    for (size_t i = 0; i < particles.size(); ++i) {
        const Eigen::Vector3d& pos = particles[i].getPosition();

        int gx = static_cast<int>(std::floor(pos.x() * m_invCellSize));
        int gy = static_cast<int>(std::floor(pos.y() * m_invCellSize));
        int gz = static_cast<int>(std::floor(pos.z() * m_invCellSize));

        int h = hashCoords(gx, gy, gz);

        m_particleHashes[i] = h;
        gridCoords[i] = {gx, gy, gz};

        m_cellStart[h]++;
    }

    int sum = 0;
    for (int i = 0; i < m_tableSize; ++i) {
        int count = m_cellStart[i];
        m_cellStart[i] = sum;
        sum += count;
    }
    m_cellStart[m_tableSize] = sum;

    m_cellOffset = m_cellStart;

    for (size_t i = 0; i < particles.size(); ++i) {
        int hash = m_particleHashes[i];
        int index = m_cellOffset[hash]++;
        m_particleIndices[index] = i;
    }

    // Record every occupied hash slot together with the grid coords of its
    // first particle (sufficient for 2x2x2 parity coloring).
    for (int slot = 0; slot < m_tableSize; ++slot) {
        if (m_cellStart[slot] < m_cellStart[slot + 1]) {
            int firstParticle = m_particleIndices[m_cellStart[slot]];
            const auto& gc = gridCoords[firstParticle];
            m_occupiedCells.push_back({slot, gc.gx, gc.gy, gc.gz});
        }
    }
}

void SpatialHash::query(const std::vector<Particle>& particles,
                        const Eigen::Vector3d& pos, double radius,
                        std::vector<int>& outNeighbors) const {
    outNeighbors.clear();
    outNeighbors.reserve(32);
    Eigen::Vector3d sphereRadius(radius, radius, radius);
    Eigen::Vector3d pMin = pos - sphereRadius;
    Eigen::Vector3d pMax = pos + sphereRadius;

    int mingx, mingy, mingz;
    int maxgx, maxgy, maxgz;

    posToGrid(pMin, mingx, mingy, mingz);
    posToGrid(pMax, maxgx, maxgy, maxgz);

    double radiusSq = radius * radius;

    for (int x = mingx; x <= maxgx; ++x) {
        for (int y = mingy; y <= maxgy; ++y) {
            for (int z = mingz; z <= maxgz; ++z) {
                int hash = hashCoords(x, y, z);
                int start = m_cellStart[hash];
                int end = m_cellStart[hash + 1];
                for (int m = start; m < end; ++m) {
                    int pIndex = m_particleIndices[m];
                    double distance =
                        (particles[pIndex].getPosition() - pos).squaredNorm();
                    if (distance < radiusSq)
                        outNeighbors.push_back(pIndex);
                }
            }
        }
    }
}

} // namespace Tissu