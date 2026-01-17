#include <omp.h>

#include "physics/Solver.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/BendingConstraint.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include <Eigen/Dense>
#include <memory>

namespace ClothSDK {
    Solver::Solver()
    : m_gravity(0.0, -9.81, 0.0), m_substeps(15), m_iterations(2), m_wind(2.0, 0.0, 1.0),
    m_airDensity(0.1), m_time(0.0), m_collisionCompliance(1e-9), m_thickness(0.08), m_spatialHash(10007, 0.08) {}

    void Solver::update(double deltaTime) {
        m_spatialHash.setCellSize(m_thickness); 
        m_spatialHash.build(m_particles);
        m_time += deltaTime;
        double substepDt = deltaTime / m_substeps;
        for (int i = 0; i < m_substeps; i++)
            step(substepDt);
    }

    void Solver::step(double dt) {
        applyForces(dt);
        predictPositions(dt);

        for (auto& constraint : m_constraints) 
            constraint->resetLambda();

        for (int i = 0; i < m_iterations; i++) {
            for(auto& constraint : m_constraints)
                constraint->solve(m_particles, dt);
        }

        for (auto& collider : m_colliders) {
            collider->resolve(m_particles, dt);
        }

        solveSelfCollisions(m_thickness);
    }

    void Solver::applyForces(double dt) {
        #pragma omp parallel for
        for(auto& particle : m_particles) {
            if(particle.getInverseMass() <= 0.0)
                continue;
            particle.addForce(m_gravity);
        }
        applyAerodynamics(dt);
        
    }

    void Solver::predictPositions(double dt) {
        #pragma omp parallel for
        for (auto& particle : m_particles) {
            particle.integrate(dt);
        }
    }

    int Solver::addParticle(const Particle& particle) {
        m_particles.push_back(particle);
        return static_cast<int>(m_particles.size() - 1);
    }

    void Solver::clear() {
        m_particles.clear();
        m_constraints.clear();
        m_colliders.clear();
    }

    const std::vector<Particle>& Solver::getParticles() const {
        return m_particles;
    }

    void Solver::addDistanceConstraint(int idA, int idB, double compliance) {
        Particle& pA = m_particles[idA];
        Particle& pB = m_particles[idB];
        double restLength = (pA.getPosition() - pB.getPosition()).norm();
        m_constraints.push_back(std::make_unique<DistanceConstraint>(idA, idB, restLength, compliance));
        m_adjacencies.insert(getAdjacencyKey(idA, idB));
    }

    void Solver::addBendingConstraint(int idA, int idB, int idC, int idD, double restAngle, double compliance) {
        m_constraints.push_back(std::make_unique<BendingConstraint>(idA, idB, idC, idD, restAngle, compliance));
        m_adjacencies.insert(getAdjacencyKey(idA, idC));
        m_adjacencies.insert(getAdjacencyKey(idB, idC));
        m_adjacencies.insert(getAdjacencyKey(idA, idD));
        m_adjacencies.insert(getAdjacencyKey(idB, idD));

    }

    void Solver::addPlaneCollider(const Eigen::Vector3d& origin, const Eigen::Vector3d& normal, double friction) {
        m_colliders.push_back(std::make_unique<PlaneCollider>(origin, normal, friction));
    }

    void Solver::addSphereCollider(const Eigen::Vector3d& center, double radius, double friction) {
        m_colliders.push_back(std::make_unique<SphereCollider>(center, radius, friction));
    }

    void Solver::addMassToParticle(int id, double mass) {
        Particle& pA = m_particles[id];
        pA.addMass(mass);
    }

    void Solver::solveConstraints(double dt) {
        for(auto& constraint : m_constraints)
            constraint->solve(m_particles, dt);
    }

    void Solver::applyAerodynamics(double dt) {
        if (dt < 1e-6) return;

        double gust = std::sin(m_time * 5.0) * 0.5 + 0.5;
        Eigen::Vector3d currentWind = m_wind * (1.0 + gust);

        #pragma omp parallel for
        for (int i = 0; i < (int)m_aeroFaces.size(); i++) {
            auto& face = m_aeroFaces[i];
            
            Particle& pA = m_particles[face.a];
            Particle& pB = m_particles[face.b];
            Particle& pC = m_particles[face.c];

            Eigen::Vector3d vFace = (pA.getVelocity(dt) + pB.getVelocity(dt) + pC.getVelocity(dt)) / 3.0;
            Eigen::Vector3d vRel = vFace - currentWind;
            double vMag = vRel.norm();
            
            if (vMag < 1e-4) continue;

            Eigen::Vector3d edge1 = pB.getPosition() - pA.getPosition();
            Eigen::Vector3d edge2 = pC.getPosition() - pA.getPosition();
            Eigen::Vector3d n = edge1.cross(edge2);
            double area = 0.5 * n.norm();
            
            if (area < 1e-6) continue;
            
            Eigen::Vector3d normal = n.normalized();

            double pressure = vRel.dot(normal) / vMag;
            Eigen::Vector3d force = -0.5 * m_airDensity * (vMag * vMag) * area * pressure * normal;
            Eigen::Vector3d forcePerVtx = force / 3.0;

            #pragma omp critical
            {
                pA.addForce(forcePerVtx);
                pB.addForce(forcePerVtx);
                pC.addForce(forcePerVtx);
            }
        }
    }


    void Solver::solveSelfCollisions(double dt) {
        double alphaHat = m_collisionCompliance / (dt * dt);
        double thicknessSq = m_thickness * m_thickness;

        for (int i = 0; i < (int)m_particles.size(); ++i) {
            Particle& pA = m_particles[i];
            double wA = pA.getInverseMass();
            if (wA == 0.0) continue;

            m_spatialHash.query(m_particles, pA.getPosition(), m_thickness, m_neighborsBuffer);

            for (int j : m_neighborsBuffer) {
                if (i >= j) continue; 

                if (m_adjacencies.count(getAdjacencyKey(i, j))) continue;

                Particle& pB = m_particles[j];
                double wB = pB.getInverseMass();
                double wSum = wA + wB;

                if (wSum + alphaHat < 1e-12) continue;

                Eigen::Vector3d dir = pA.getPosition() - pB.getPosition();
                double distSq = dir.squaredNorm();

                if (distSq > 0.0 && distSq < thicknessSq) {
                    double dist = std::sqrt(distSq);
                    Eigen::Vector3d normal = dir / dist;

                    double C = dist - m_thickness;
                    
                    double deltaLambda = -C / (wSum + alphaHat);
                    Eigen::Vector3d corr = normal * deltaLambda;

                    pA.setPosition(pA.getPosition() + corr * wA);
                    pB.setPosition(pB.getPosition() - corr * wB);
                }
            }
        }
    }

    uint64_t Solver::getAdjacencyKey(int idA, int idB) const{
        uint64_t low = static_cast<uint32_t>(std::min(idA, idB));
        uint64_t high = static_cast<uint32_t>(std::max(idA, idB));

        return (high << 32) | low;
    }

    void Solver::setIterations(int count) {
        m_iterations = count;
    }

    void Solver::setSubsteps(int count) {
        m_substeps = count;
    }

    void Solver::setGravity(const Eigen::Vector3d& gravity) {
        m_gravity = gravity;
    }

    void Solver::setParticleInverseMass(int id, double invMass) {
        m_particles[id].setInverseMass(invMass);
    }

    void Solver::addAeroFace(int idA, int idB, int idC) {
        m_aeroFaces.push_back({idA, idB, idC});
    }
}
