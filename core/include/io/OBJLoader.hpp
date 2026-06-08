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
#include <Eigen/Dense>
#include <string>
#include <vector>

namespace Tissu {

/**
 * @class OBJLoader
 * @brief Parses Wavefront OBJ files into solver-ready geometry buffers.
 *
 * Wraps tinyobjloader to extract vertex positions and triangle indices
 * from an OBJ file. The output is intended to be passed directly to
 * @ref ClothMesh::buildFromMesh.
 */
class OBJLoader {
public:
  /**
   * @brief Loads vertex positions and triangle indices from a Wavefront OBJ
   * file.
   *
   * @param path Path to the .obj file.
   * @param outPos Output vector populated with one entry per unique vertex.
   *        Cleared before writing.
   * @param outIndices Output vector of flat triangle indices into @p outPos.
   *        Cleared before writing.
   * @return true If correctly loaded.
   * @return false In other way.
   */
  static bool load(const std::string &path,
                   std::vector<Eigen::Vector3d> &outPos,
                   std::vector<int> &outIndices);
};

} // namespace Tissu
