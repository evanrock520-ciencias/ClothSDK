// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "physics/GravityForce.hpp"

namespace Tissu {

void GravityForce::apply(std::vector<Particle> &particles, double dt) {
#pragma omp parallel for
  for (int i = 0; i < (int)particles.size(); i++) {
    if (particles[i].getInverseMass() == 0.0)
      continue;

    particles[i].addForce(m_gravity);
  }
}

} // namespace Tissu