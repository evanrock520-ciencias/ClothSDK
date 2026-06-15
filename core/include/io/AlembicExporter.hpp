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
#include <memory>
#include <string>
#include <vector>

namespace Tissu {

/**
 * @class AlembicExporter
 * @brief Exporter for the Alembic (.abc) format.
 *
 */
class AlembicExporter {
 public:
  AlembicExporter();
  ~AlembicExporter();

  /**
   * @brief Creates a new .abc file and initializes the mesh topology.
   * @param path Target filesystem path.
   * @param positions Initial vertex positions to define the count.
   * @param indices Triangle indices defining the fixed topology.
   * @return true if the file was successfully created.
   */
  bool open(const std::string& path,
            const std::vector<std::string>& names,
            const std::vector<Eigen::Vector3d>& global_positions,
            const std::vector<std::vector<int>>& global_indices,
            const std::vector<std::vector<int>>& particle_indices);

  /**
   * @brief Writes a single simulation frame to the archive.
   * @param global_positions Current global vertex positions from the solver.
   * @param time The timestamp for this frame.
   */
  void writeFrame(const std::vector<Eigen::Vector3d>& global_positions, double time);

  /**
   * @brief Finalizes the archive and closes the file.
   */
  void close();

 private:
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

}  // namespace Tissu