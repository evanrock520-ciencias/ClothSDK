#include <gtest/gtest.h>

#include <vector>

#include "physics/Particle.hpp"
#include "physics/PinConstraint.hpp"

using namespace Tissu;

TEST(PinConstraint, ZeroComplianceMovesParticleToPinPositionInOneIteration) {
    Eigen::Vector3d pinPos(10.0, 1.0, 8.0);
    Eigen::Vector3d initialPos(0.0, 2.0, 3.0);

    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    PinConstraint constraint(0, pinPos, 0.0);
    constraint.solve(particles, 0.016);

    ASSERT_EQ(particles[0].getPosition().x(), pinPos.x());
    ASSERT_EQ(particles[0].getPosition().y(), pinPos.y());
    ASSERT_EQ(particles[0].getPosition().z(), pinPos.z());
}

TEST(PinConstraint, StaticParticleStaysInTheSamePosition) {
    Eigen::Vector3d pinPos(10.0, 1.0, 8.0);
    Eigen::Vector3d initialPos(0.0, 2.0, 3.0);

    std::vector<Particle> particles;
    particles.emplace_back(initialPos);
    particles[0].setInverseMass(0.0);

    PinConstraint constraint(0, pinPos, 0.0);
    constraint.solve(particles, 0.016);

    ASSERT_EQ(particles[0].getPosition().x(), initialPos.x());
    ASSERT_EQ(particles[0].getPosition().y(), initialPos.y());
    ASSERT_EQ(particles[0].getPosition().z(), initialPos.z());
}

TEST(PinConstraint, ConvergeToPinPosition) {
    Eigen::Vector3d pinPos(10.0, 1.0, 8.0);
    Eigen::Vector3d initialPos(0.0, 2.0, 3.0);

    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    PinConstraint constraint(0, pinPos, 0.0);
    int iterations = 100;

    for (int idx = 0; idx < iterations; idx++)
        constraint.solve(particles, 0.016);

    ASSERT_NEAR(particles[0].getPosition().x(), pinPos.x(), 1e-4);
    ASSERT_NEAR(particles[0].getPosition().y(), pinPos.y(), 1e-4);
    ASSERT_NEAR(particles[0].getPosition().z(), pinPos.z(), 1e-4);
}

TEST(PinConstraint, LowerComplianceConvergeFasterToPinPosition) {
    Eigen::Vector3d pinPos(10.0, 12.0, 8.0);
    std::vector<Particle> particles;

    particles.emplace_back(Eigen::Vector3d(4.0, 4.0, 4.0));
    particles.emplace_back(Eigen::Vector3d(4.0, 4.0, 4.0));

    int iterations = 5;

    PinConstraint constraintA(0, pinPos, 0.2);
    PinConstraint constraintB(1, pinPos, 0.8);

    for (int idx = 0; idx < iterations; idx++)
        constraintA.solve(particles, 0.016);

    for (int idx = 0; idx < iterations; idx++)
        constraintB.solve(particles, 0.016);

    double distanceA = (pinPos - particles[0].getPosition()).norm();
    double distanceB = (pinPos - particles[1].getPosition()).norm();

    ASSERT_LT(distanceA, distanceB);
}