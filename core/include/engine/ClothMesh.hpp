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

#include "math/Types.hpp"
#include <Eigen/Dense>
#include <map>
#include <vector>
#include <memory>

namespace Tissu {

class Solver;
class Cloth; 

class ClothMesh {
public:
    ClothMesh() = default;

    void initGrid(int rows, int cols, double spacing, Cloth& outCloth, Solver& solver);

    void buildFromMesh(const std::vector<Eigen::Vector3d>& positions, 
                        const std::vector<int>& indices, 
                        Cloth& outCloth, 
                        Solver& solver,
                        const std::string& meshPath);

private:
    bool isClosed() const;

    int getOppositeVertex(const Triangle& tri, int v1, int v2) const;
    double calculateInitialAngle(int id1, int id2, int id3, int id4, const Solver& solver) const;
    
    void computePhysicalAttributes(Cloth& cloth, Solver& solver) const;

};

} 