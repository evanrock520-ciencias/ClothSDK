#include "engine/ClothMesh.hpp"
#include "physics/Solver.hpp"
#include <fstream>

namespace ClothSDK {

ClothMesh::ClothMesh() 
: m_density(1.0), m_structuralCompliance(0.8), m_shearCompliance(0.5), m_bendingCompliance(0.2), m_cols(0), m_rows(0) {} 

void ClothMesh::initGrid(int rows, int cols, double spacing, Solver& solver) {
    m_rows = rows;
    m_cols = cols;
    m_triangles.clear();

    for(int r = 0; r < m_rows; r++) {
        for(int c = 0; c < m_cols; c++) {
            Eigen::Vector3d pos(c * spacing, r * spacing, 0.0);
            int id = solver.addParticle(Particle(pos));
            m_particlesIndices.push_back(id);
        }
    }

    for(int r = 0; r < m_rows; r++) {
        for (int c = 0; c < m_cols; c++) {
            if (c < cols - 1) {
                int idA = getParticleID(r, c);
                int idB = getParticleID(r, c + 1);
                solver.addDistanceConstraint(idA, idB, m_structuralCompliance);
            }

            if (r < rows - 1) {
                int idA = getParticleID(r, c);
                int idB = getParticleID(r + 1, c);
                solver.addDistanceConstraint(idA, idB, m_structuralCompliance);
            }

            if (r < rows - 1 && c < cols - 1) {
                int idA = getParticleID(r, c);
                int idB = getParticleID(r, c + 1);
                int idC = getParticleID(r + 1, c);
                int idD = getParticleID(r + 1, c + 1);
                solver.addDistanceConstraint(idA, idD, m_shearCompliance);
                solver.addDistanceConstraint(idB, idC, m_shearCompliance);

                solver.addBendingConstraint(idA, idD, idB, idC, 0.0, m_bendingCompliance);

                m_triangles.push_back(Triangle({idA, idB, idD}));
                m_triangles.push_back(Triangle({idA, idD, idC}));                
            }
        }
    }

    const std::vector<Particle>& particles = solver.getParticles();

    for(auto& triangle : m_triangles) {
        const Particle& pA = particles[triangle.a];
        const Particle& pB = particles[triangle.b];
        const Particle& pC = particles[triangle.c];

        Eigen::Vector3d vA = pB.getPosition() - pA.getPosition();
        Eigen::Vector3d vB = pC.getPosition() - pA.getPosition();

        double area = 0.5 * vA.cross(vB).norm();
        double massPerVertex = (area * m_density) / 3.0;

        solver.addMassToParticle(triangle.a, massPerVertex);
        solver.addMassToParticle(triangle.b, massPerVertex);
        solver.addMassToParticle(triangle.c, massPerVertex);
        solver.addAeroFace(triangle.a, triangle.b, triangle.c);
    }
}

void ClothMesh::setMaterial(double density, double stretch, double shear, double bend) {
    m_density = density;
    m_structuralCompliance = stretch;
    m_shearCompliance = shear;
    m_bendingCompliance = bend;
}

int ClothMesh::getParticleID(int row, int col) const {
    int localIndex = row * m_cols + col;
    return m_particlesIndices[localIndex];
}

void ClothMesh::exportToOBJ(const std::string& filename, const Solver& solver) const {
    std::ofstream file(filename);
    if (!file.is_open()) return;

    const std::vector<Particle>& particles = solver.getParticles();

    for (int id : m_particlesIndices) {
        const Eigen::Vector3d& pos = particles[id].getPosition();
        
        file << "v " << pos.x() << " " << pos.y() << " " << pos.z() << "\n";
    }

    for (const auto& t : m_triangles) {
        file << "f " << t.a + 1 << " " << t.b + 1 << " " << t.c + 1 << "\n";
    }

    file.close();
}
}