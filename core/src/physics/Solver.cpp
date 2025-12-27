#include "physics/Solver.hpp"
#include <vector>

namespace ClothSDK {
    Solver::Solver() : m_gravity(0.0, -9.81, 0.0), m_substeps(2) {}

    void Solver::update(double deltaTime) {
        double substepDt = deltaTime / m_substeps;
        for (int i = 0; i < m_substeps; i++)
            step(substepDt);
    }

    void Solver::step(double dt) {
        applyForces();
        predictPositions(dt);
        // Solve constrains
    }

    void Solver::applyForces() {
        for(auto& particle : listParticles) {
            if(particle.getInverseMass() <= 0.0)
                continue;
            particle.addForce(m_gravity * (1.0 / particle.getInverseMass()));
        }
    }

    void Solver::predictPositions(double dt) {
        for (auto& particle : listParticles) {
            particle.integrate(dt);
        }
    }

    void Solver::addParticle(const Particle& particle) {
        listParticles.push_back(particle);
    }

    void Solver::clear() {
        listParticles.clear();
    }

    const std::vector<Particle>& Solver::getParticles() const {
        return listParticles;
    }
}