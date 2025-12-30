#pragma once

#include "math/Types.hpp"

#include <Eigen/Dense>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

namespace ClothSDK {

class Solver;

class ClothMesh {
public:
    ClothMesh();

    void initGrid(int rows, int cols, double spacing, Solver& solver);
    void buildFromMesh(const std::vector<Eigen::Vector3d>& positions, const std::vector<int>& indexs, Solver& solver);

    void setMaterial(double density, double stretch, double shear, double bend);

    void exportToOBJ(const std::string& filename, const Solver& solver) const;

    int getParticleID(int row, int col) const;

private:
    std::vector<int> m_particlesIndices;
    std::vector<Triangle> m_triangles;

    double m_density;
    double m_structuralStiffness;
    double m_shearStiffness;
    double m_bendingStiffness;

    int m_rows, m_cols;
};

}