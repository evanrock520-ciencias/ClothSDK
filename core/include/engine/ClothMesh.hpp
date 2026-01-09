#pragma once

#include "math/Types.hpp"

#include <Eigen/Dense>
#include <cmath>
#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include <map>

namespace ClothSDK {

class Solver;

class ClothMesh {
public:
    ClothMesh();

    void initGrid(int rows, int cols, double spacing, Solver& solver);
    void buildFromMesh(const std::vector<Eigen::Vector3d>& positions, const std::vector<int>& indices, Solver& solver);

    void setMaterial(double density, double stretch, double shear, double bend);

    void exportToOBJ(const std::string& filename, const Solver& solver) const;

    int getParticleID(int row, int col) const;

    double getDensity() const { return m_density; }
    double getStructuralCompliance() const { return m_structuralCompliance; }
    double getShearCompliance() const { return m_shearCompliance; }
    double getBendingCompliance() const { return m_bendingCompliance; }

private:
    struct Edge {
        int v1, v2;
        Edge(int a, int b) : v1(std::min(a, b)), v2(std::max(a, b)) {}
        
        bool operator<(const Edge& other) const {
            return v1 < other.v1 || (v1 == other.v1 && v2 < other.v2);
        }
    };

    int getOppositeVertex(const Triangle& tri, int v1, int v2) const;
    double calculateInitialAngle(int v1,int v2,int v3,int v4, const Solver& solver) const;

    std::vector<int> m_particlesIndices;
    std::vector<Triangle> m_triangles;

    double m_density;
    double m_structuralCompliance;
    double m_shearCompliance;
    double m_bendingCompliance;

    int m_rows, m_cols;
};

}