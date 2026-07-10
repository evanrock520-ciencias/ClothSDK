#pragma once
#include "physics/Constraint.hpp"
#include "physics/Particle.hpp"

namespace Tissu {

class StitchConstraint : public Constraint {
public:
    StitchConstraint(int idA, int idB, double compliance);

    void solve(std::vector<Particle>& particles, double dt) override;
    std::vector<int> getParticleIds() const override { return {m_idA, m_idB}; }

private:
    int m_idA;
    int m_idB;
};

} // namespace Tissu