#include "engine/ClothMesh.hpp"
#include "physics/Solver.hpp"
#include "physics/Particle.hpp"
#include <cmath>
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

void ClothMesh::buildFromMesh(const std::vector<Eigen::Vector3d>& positions, const std::vector<int>& indices, Solver& solver) {
    std::map<Edge, std::vector<int>> edgeToTriangles;
    m_particlesIndices.clear();
    m_triangles.clear();

    for (auto& position : positions) {
        auto id = solver.addParticle(Particle(position));
        m_particlesIndices.push_back(id);
    }

    std::vector<int> solver_indices;
    solver_indices.reserve(indices.size());

    for (const auto& indice : indices) {
        solver_indices.push_back(m_particlesIndices[indice]);
    }

    for (size_t i = 0; i < solver_indices.size(); i += 3) {
        auto vA = solver_indices[i];
        auto vB = solver_indices[i + 1];
        auto vC = solver_indices[i + 2];

        m_triangles.push_back({vA, vB, vC});
        int id = m_triangles.size() - 1;

        Edge edges[3] = { {vA, vB}, {vB, vC}, {vC, vA} };

        for (auto& edge : edges) {
            if (edgeToTriangles.find(edge) == edgeToTriangles.end()) 
                solver.addDistanceConstraint(edge.v1, edge.v2, m_structuralCompliance);
            edgeToTriangles[edge].push_back(id);
        }
    }

    for (auto const& [key, triList] : edgeToTriangles) {
        if (triList.size() == 2) {
            int v1 = key.v1;
            int v2 = key.v2;

            const Triangle& t1 = m_triangles[triList[0]];
            const Triangle& t2 = m_triangles[triList[1]];

            int v3 = getOppositeVertex(t1, v1, v2);
            int v4 = getOppositeVertex(t2, v1, v2);

            double initialAngle = calculateInitialAngle(v1, v2, v3, v4, solver);

            solver.addBendingConstraint(v1, v2, v3, v4, initialAngle, m_bendingCompliance);
        }
    }

    const auto& particles = solver.getParticles();

    for (int id : m_particlesIndices) {
        solver.setParticleInverseMass(id, 1e6); 
    }

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

int ClothMesh::getOppositeVertex(const Triangle& tri, int v1, int v2) const{
    if (tri.a != v1 && tri.a != v2) return tri.a;
    if (tri.b != v1 && tri.b != v2) return tri.b;
    return tri.c;
}

double ClothMesh::calculateInitialAngle(int id1, int id2, int id3, int id4, const Solver& solver) const {
    const auto& particles = solver.getParticles();
    
    const Eigen::Vector3d& p1 = particles[id1].getPosition();
    const Eigen::Vector3d& p2 = particles[id2].getPosition(); 
    const Eigen::Vector3d& p3 = particles[id3].getPosition(); 
    const Eigen::Vector3d& p4 = particles[id4].getPosition(); 

    Eigen::Vector3d e = p2 - p1;
    if (e.isZero(1e-6)) return 0.0; 

    Eigen::Vector3d n1 = e.cross(p3 - p1);
    Eigen::Vector3d n2 = (p4 - p1).cross(e); 

    double len1 = n1.norm();
    double len2 = n2.norm();

    if (len1 < 1e-6 || len2 < 1e-6) return 0.0; 

    double cosTheta = n1.dot(n2) / (len1 * len2);
    
    return std::acos(std::clamp(cosTheta, -1.0, 1.0));
}


}