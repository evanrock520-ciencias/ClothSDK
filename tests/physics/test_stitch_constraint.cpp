#include <vector>

#include "Eigen/Dense"
#include "physics/Particle.hpp"
#include "physics/Solver.hpp"
#include "physics/StitchConstraint.hpp"
#include "gtest/gtest.h"

using namespace Tissu;

TEST(StitchConstraint, ParticleShareSamePosition) {
    Solver solver;
    std::vector<Particle> particles;
    particles.push_back(Particle(Eigen::Vector3d(0.0, 0.0, 0.0)));
    particles.push_back(Particle(Eigen::Vector3d(0.0, 6.0, 0.0)));
    StitchConstraint constraint(0, 1, 0.0);

    int iterations = 100;

    for (int idx = 0; idx < iterations; idx++)
        constraint.solve(particles, 0.016);

    double distance =
        (particles[0].getPosition() - particles[1].getPosition()).norm();
    EXPECT_NEAR(0.0, distance, 1e-6);
    EXPECT_NEAR(0.0, particles[0].getPosition().x(), 1e-6);
    EXPECT_NEAR(0.0, particles[1].getPosition().x(), 1e-6);
}

TEST(StitchConstraint, StaticParticlePullsDynamicParticle) {
    Solver solver;
    std::vector<Particle> particles;
    particles.push_back(Particle(Eigen::Vector3d(0.0, 0.0, 0.0)));
    particles.push_back(Particle(Eigen::Vector3d(0.0, 6.0, 0.0)));
    particles[0].setInverseMass(0.0);
    StitchConstraint constraint(0, 1, 0.0);

    int iterations = 100;

    for (int idx = 0; idx < iterations; idx++)
        constraint.solve(particles, 0.016);

    EXPECT_NEAR(particles[1].getPosition().y(), 0.0, 1e-6);
    EXPECT_EQ(particles[0].getPosition().y(), 0.0);
}

TEST(StitchConstraint, LowerComplianceConvergesFaster) {
    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(-4.0, 0.0, 0.0));
    particles.emplace_back(Eigen::Vector3d(0.0, 0.0, 0.0));

    particles.emplace_back(Eigen::Vector3d(4.0, 0.0, 0.0));
    particles.emplace_back(Eigen::Vector3d(8.0, 0.0, 0.0));

    int iterations = 5;

    StitchConstraint constraintLC(0, 1, 0.0);
    StitchConstraint constraintGC(2, 3, 0.8);

    for (int idx = 0; idx < iterations; idx++) {
        constraintLC.solve(particles, 0.016);
        constraintGC.solve(particles, 0.016);
    }

    double distanceLC =
        (particles[0].getPosition() - particles[1].getPosition()).norm();
    double distanceGC =
        (particles[2].getPosition() - particles[3].getPosition()).norm();

    EXPECT_LT(distanceLC, distanceGC);
}