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
#include <vector>
#include <Eigen/Dense>

namespace Tissu {

class Particle;

/**
 * @class SpatialHash
 * @brief Uniform grid spatial acceleration structure for neighbor queries.
 *
 * Partitions 3D space into a fixed-size grid and maps each cell to a slot
 * in a flat hash table using a prime-multiplier XOR hash. This allows
 * O(1) average-case insertion and neighborhood queries without dynamic
 * memory allocation per cell.
 *
 * The structure must be rebuilt every substep via @ref build before
 * calling @ref query, since particle positions change each iteration.
 * It is primarily used by @ref Solver::solveSelfCollisions to find
 * candidate particle pairs within a given proximity threshold.
 *
 * @note Hash collisions are possible and intentionally tolerated —
 * false positives are filtered by the distance check in @ref query,
 * and false negatives are rare enough to be acceptable for real-time use.
 */
class SpatialHash {
public:
    /**
     * @brief Constructs a SpatialHash with a fixed table size and initial cell size.
     *
     * @param tableSize Number of slots in the hash table. Should be a large
     *                  prime to minimize collisions.
     * @param cellSize  Side length of each grid cell in world units.
     *                  Should match the expected query radius for best performance.
     *                  Can be updated each frame via @ref setCellSize.
     */
    SpatialHash(int tableSize, double cellSize);

    /**
     * @brief Rebuilds the acceleration structure from the current particle positions.
     *
     * Performs a counting sort to populate the internal cell-start and
     * particle-index arrays. Must be called once per substep before any
     * @ref query calls.
     *
     * @param particles Reference to the solver's global particle buffer.
     */
    void build(const std::vector<Particle>& particles);

    /**
     * @brief Returns the indices of all particles within a sphere.
     *
     * Iterates over all grid cells overlapping the query sphere's AABB
     * and collects particles whose squared distance to @p pos is less
     * than @p radius². Results are written to @p outNeighbors, which is
     * cleared at the start of each call.
     *
     * @param particles     Reference to the solver's global particle buffer.
     * @param pos           Center of the query sphere in world space.
     * @param radius        Search radius in world units.
     * @param outNeighbors  Output vector populated with matching particle indices.
     */
    void query(const std::vector<Particle>& particles, const Eigen::Vector3d& pos, double radius, std::vector<int>& outNeighbors) const;

    /**
     * @brief Updates the grid cell size.
     *
     * Should be called when the simulation's collision thickness changes,
     * since the optimal cell size equals the query radius.
     *
     * @param h New cell side length in world units.
     */
    void setCellSize(double h) { m_cellSize = h; }

    /** @return Current grid cell side length in world units. */
    double getCellSize() const { return m_cellSize; }

private:
    /**
     * @brief Maps a 3D grid coordinate to a hash table slot.
     *
     * Uses a prime-multiplier XOR hash to distribute coordinates
     * uniformly across the table. Collisions are possible but rare
     * with a sufficiently large @ref m_tableSize.
     *
     * @param x Grid coordinate on the X axis.
     * @param y Grid coordinate on the Y axis.
     * @param z Grid coordinate on the Z axis.
     * @return  Hash table index in the range [0, m_tableSize).
     */
    inline int hashCoords(int x, int y, int z) const {
        unsigned int h = (static_cast<unsigned int>(x) * 73856093) ^
                         (static_cast<unsigned int>(y) * 19349663) ^
                         (static_cast<unsigned int>(z) * 83492791);
        return static_cast<int>(h % m_tableSize);
    }

    /**
     * @brief Converts a world-space position to integer grid coordinates.
     *
     * @param pos World-space point to convert.
     * @param gx  Output grid coordinate on the X axis.
     * @param gy  Output grid coordinate on the Y axis.
     * @param gz  Output grid coordinate on the Z axis.
     */
    inline void posToGrid(const Eigen::Vector3d& pos, int& gx, int& gy, int& gz) const {
        gx = static_cast<int>(std::floor(pos.x() / m_cellSize));
        gy = static_cast<int>(std::floor(pos.y() / m_cellSize));
        gz = static_cast<int>(std::floor(pos.z() / m_cellSize));
    }

    int m_tableSize;                    ///< Number of slots in the hash table.
    double m_cellSize;                  ///< Side length of each grid cell in world units.
    std::vector<int> m_cellStart;       ///< Prefix-sum array mapping each hash slot to its first entry in @ref m_particleIndices.
    std::vector<int> m_particleIndices; ///< Particle indices sorted by their hash slot, built during @ref build.
    std::vector<int> m_particleHashes;  ///< Hash slot assigned to each particle, indexed by particle ID.
};

} 