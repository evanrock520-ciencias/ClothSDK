// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/OBJExporter.hpp"
#include "engine/Cloth.hpp"    
#include "physics/Solver.hpp"
#include <fstream>

namespace ClothSDK {

void OBJExporter::exportOBJ(const std::string &filename, const Cloth& cloth, const Solver &solver) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return;
    }

    const std::vector<Particle>& allParticles = solver.getParticles();
    
    const std::vector<int>& pIndices = cloth.getParticleIndices();
    for (int id : pIndices) {
        const Eigen::Vector3d& pos = allParticles[id].getPosition();
        file << "v " << pos.x() << " " << pos.y() << " " << pos.z() << "\n";
    }

    const auto& triangles = cloth.getTriangles();
    for (const auto& t : triangles) {
        auto getRelativeIndex = [&](int globalId) {
            for (size_t i = 0; i < pIndices.size(); ++i) {
                if (pIndices[i] == globalId) return static_cast<int>(i) + 1;
            }
            return 1; 
        };

        file << "f " << getRelativeIndex(t.a) << " " 
                << getRelativeIndex(t.b) << " " 
                << getRelativeIndex(t.c) << "\n";
    }

    file.close();
}

}
