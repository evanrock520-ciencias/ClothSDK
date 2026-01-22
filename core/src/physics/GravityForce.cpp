#include "physics/GravityForce.hpp"

namespace ClothSDK {

void GravityForce::apply(std::vector<Particle>& particles, double dt) {
    #pragma omp parallel for
    for (auto& p : particles) {
        if (p.getInverseMass() == 0.0)
            continue;

        p.addForce(m_gravity);
    }
}

}