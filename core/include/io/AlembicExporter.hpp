#pragma once

#include <string>
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace ClothSDK {

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
              const std::vector<Eigen::Vector3d>& positions, 
              const std::vector<int>& indices);

    /**
     * @brief Writes a single simulation frame to the archive.
     * @param positions Current vertex positions from the solver.
     * @param time The timestamp for this frame.
     */
    void writeFrame(const std::vector<Eigen::Vector3d>& positions, double time);

    /**
     * @brief Finalizes the archive and closes the file.
     */
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}