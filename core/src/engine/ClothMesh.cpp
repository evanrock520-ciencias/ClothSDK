// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "engine/ClothMesh.hpp"
#include "engine/Cloth.hpp"
#include "math/Types.hpp"
#include "physics/Solver.hpp"
#include "physics/Particle.hpp"
#include <cmath>
#include <fstream>
#include <map>
#include <vector>

namespace ClothSDK {

void ClothMesh::initGrid(int rows, int cols, double spacing, Cloth& outCloth, Solver& solver) {
    outCloth.setGridDimensions(rows, cols);
    std::vector<int> gridIndices;
    gridIndices.reserve(rows * cols);   
    auto mat = outCloth.getMaterial();
    double stComp = mat->structuralCompliance;
    double shComp = mat->shearCompliance;
    double beComp = mat->bendingCompliance;
    double dens = mat->density;
    outCloth.clear();

    auto getLocalID = [&](int r, int c) {
        return gridIndices[r * cols + c];
    };
    
    for(int r = 0; r < rows; r++) {
        for(int c = 0; c < cols; c++) {
            Eigen::Vector3d pos(c * spacing, r * spacing, 0.0);
            int id = solver.addParticle(Particle(pos));
            gridIndices.push_back(id);
            outCloth.addParticleId(id);
        }
    }

    for(int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (c < cols - 1) {
                int idA = getLocalID(r, c);
                int idB = getLocalID(r, c + 1);
                solver.addDistanceConstraint(idA, idB, stComp);
                outCloth.addVisualEdge(idA, idB);
            }

            if (r < rows - 1) {
                int idA = getLocalID(r, c);
                int idB = getLocalID(r + 1, c);
                solver.addDistanceConstraint(idA, idB, stComp);
                outCloth.addVisualEdge(idA, idB);
            }

            if (r < rows - 1 && c < cols - 1) {
                int idA = getLocalID(r, c);
                int idB = getLocalID(r, c + 1);
                int idC = getLocalID(r + 1, c);
                int idD = getLocalID(r + 1, c + 1);
                solver.addDistanceConstraint(idA, idD, shComp);
                solver.addDistanceConstraint(idB, idC, shComp);

                solver.addBendingConstraint(idA, idD, idB, idC, 0.0, beComp);

                outCloth.addVisualEdge(idA, idD);
                outCloth.addVisualEdge(idB, idC);

                outCloth.addTriangle(Triangle{idA, idB, idD});
                outCloth.addTriangle(Triangle({idA, idD, idC}));                
            }
        }
    }

    computePhysicalAttributes(outCloth, solver);
}

void ClothMesh::buildFromMesh(const std::vector<Eigen::Vector3d>& positions, const std::vector<int>& indices, Cloth& outCloth, Solver& solver) {
    std::map<Edge, std::vector<int>> edgeToTriangles;
    std::vector<int> localToGlobal; 
    localToGlobal.reserve(positions.size());
    outCloth.clear();
    auto mat = outCloth.getMaterial();
    double stComp = mat->structuralCompliance;
    double shComp = mat->shearCompliance;
    double beComp = mat->bendingCompliance;
    double dens = mat->density;


    for (auto& position : positions) {
        auto id = solver.addParticle(Particle(position));
        outCloth.addParticleId(id);
        localToGlobal.push_back(id);
    }

    for (size_t i = 0; i < indices.size(); i += 3) {
        int vA = localToGlobal[indices[i]];
        int vB = localToGlobal[indices[i+1]];
        int vC = localToGlobal[indices[i+2]];

        outCloth.addTriangle({vA, vB, vC});
        
        int currentTriId = outCloth.getTriangles().size() - 1;

        Edge edges[3] = { {vA, vB}, {vB, vC}, {vC, vA} };
        for (auto& edge : edges) {
            if (edgeToTriangles.find(edge) == edgeToTriangles.end()) {
                solver.addDistanceConstraint(edge.v1, edge.v2, stComp);
                outCloth.addVisualEdge(edge.v1, edge.v2); 
            }
            edgeToTriangles[edge].push_back(currentTriId);
        }
    }

    for (auto const& [key, triList] : edgeToTriangles) {
        if (triList.size() == 2) {
            int v1 = key.v1;
            int v2 = key.v2;

            const Triangle& t1 = outCloth.getTriangles()[triList[0]];
            const Triangle& t2 = outCloth.getTriangles()[triList[1]];

            int v3 = getOppositeVertex(t1, v1, v2);
            int v4 = getOppositeVertex(t2, v1, v2);

            double initialAngle = calculateInitialAngle(v1, v2, v3, v4, solver);

            solver.addBendingConstraint(v1, v2, v3, v4, initialAngle, beComp);
        }
    }

    computePhysicalAttributes(outCloth, solver);
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

void ClothMesh::computePhysicalAttributes(Cloth& cloth, Solver& solver) const {
    const auto& particles = solver.getParticles();
    const auto& triangles = cloth.getTriangles();
    const auto& indices = cloth.getParticleIndices();
    double density = cloth.getMaterial()->density;

    for(const auto& triangle : triangles) {
        const Particle& pA = particles[triangle.a];
        const Particle& pB = particles[triangle.b];
        const Particle& pC = particles[triangle.c];

        Eigen::Vector3d v1 = pB.getPosition() - pA.getPosition();
        Eigen::Vector3d v2 = pC.getPosition() - pA.getPosition();

        double area = 0.5 * v1.cross(v2).norm();
        double massPerVertex = (area * density) / 3.0;

        solver.addMassToParticle(triangle.a, massPerVertex);
        solver.addMassToParticle(triangle.b, massPerVertex);
        solver.addMassToParticle(triangle.c, massPerVertex);
        
        cloth.addAeroFace(triangle.a, triangle.b, triangle.c);
    }
}


}