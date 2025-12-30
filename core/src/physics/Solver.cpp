#include "physics/Solver.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include <memory>
#include <vector>

namespace ClothSDK {
    Solver::Solver() : m_gravity(0.0, -9.81, 0.0), m_substeps(2), m_iterations(5) {}

    void Solver::update(double deltaTime) {
        double substepDt = deltaTime / m_substeps;
        for (int i = 0; i < m_substeps; i++)
            step(substepDt);
    }

    void Solver::step(double dt) {
        applyForces();
        predictPositions(dt);
        for (auto& constraint : m_constraints) 
            constraint->resetLambda();
        solveConstraints(dt);
        for (auto& collider : m_colliders) {
            collider->resolve(m_particles, dt);
        }
    }

    void Solver::applyForces() {
        for(auto& particle : m_particles) {
            if(particle.getInverseMass() <= 0.0)
                continue;
            particle.addForce(m_gravity * (1.0 / particle.getInverseMass()));
        }
    }

    void Solver::predictPositions(double dt) {
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
        for (int i = 0; i < m_iterations; i++) {
            for(auto& constraint : m_constraints)
                constraint->solve(m_particles, dt);
        }
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
}