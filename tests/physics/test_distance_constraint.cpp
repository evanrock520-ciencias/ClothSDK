#include <gtest/gtest.h>

#include "physics/DistanceConstraint.hpp"
#include "physics/Particle.hpp"

using namespace Tissu;

TEST(DistanceConstraint, ConvergeToRestLenght) {
  std::vector<Particle> particles;
  particles.emplace_back(Eigen::Vector3d(0.0, 0.0, 0.0));
  particles.emplace_back(Eigen::Vector3d(2.0, 0.0, 0.0));

  int iterations = 100;

  DistanceConstraint constraint(0, 1, 1.0, 0.0);
  for (int idx = 0; idx < iterations; idx++) constraint.solve(particles, 0.016);

  double distance =
      (particles[0].getPosition() - particles[1].getPosition()).norm();

  EXPECT_NEAR(1.0, distance, 1e-9);
}

TEST(DistanceConstraint, StaticParticlesDoesNotChangeThemPositions) {
  Eigen::Vector3d initialP1(4.0, 4.0, 4.0);
  Eigen::Vector3d initialP2(0.0, 0.0, 0.0);

  Particle p1(initialP1);
  Particle p2(initialP2);
  p1.setInverseMass(0.0);
  p2.setInverseMass(0.0);

  std::vector<Particle> particles;

  particles.emplace_back(p1);
  particles.emplace_back(p2);

  int iterations = 100;

  DistanceConstraint constraint(0, 1, 1.0, 0.0);
  for (int idx = 0; idx < iterations; idx++) constraint.solve(particles, 0.016);

  EXPECT_EQ(particles[0].getPosition().x(), initialP1.x());
  EXPECT_EQ(particles[0].getPosition().y(), initialP1.y());
  EXPECT_EQ(particles[0].getPosition().z(), initialP1.z());

  EXPECT_EQ(particles[1].getPosition().x(), initialP2.x());
  EXPECT_EQ(particles[1].getPosition().y(), initialP2.y());
  EXPECT_EQ(particles[1].getPosition().z(), initialP2.z());
}

TEST(DistanceConstraint,
     StaticParticleDoesNotChangeItsPositionButNonStaticDoIt) {
  Eigen::Vector3d initialP1(4.0, 4.0, 4.0);
  Eigen::Vector3d initialP2(0.0, 0.0, 0.0);

  Particle p1(initialP1);
  Particle p2(initialP2);
  p1.setInverseMass(0.0);
  p2.setInverseMass(1.0);

  std::vector<Particle> particles;

  particles.emplace_back(p1);
  particles.emplace_back(p2);

  int iterations = 100;

  DistanceConstraint constraint(0, 1, 1.0, 0.0);
  for (int idx = 0; idx < iterations; idx++) constraint.solve(particles, 0.016);

  double distance =
      (particles[0].getPosition() - particles[1].getPosition()).norm();
  EXPECT_NEAR(distance, 1.0, 1e-4);

  EXPECT_EQ(particles[0].getPosition().x(), initialP1.x());
  EXPECT_EQ(particles[0].getPosition().y(), initialP1.y());
  EXPECT_EQ(particles[0].getPosition().z(), initialP1.z());
}

TEST(
    DistanceConstraint,
    StaticParticleAndNonStaticParticleWithZeroComplianceConvergeToRestLenghtInOneIteration) {
  Eigen::Vector3d initialP1(4.0, 4.0, 4.0);
  Eigen::Vector3d initialP2(0.0, 0.0, 0.0);

  Particle p1(initialP1);
  Particle p2(initialP2);
  p1.setInverseMass(0.0);
  p2.setInverseMass(1.0);

  std::vector<Particle> particles;

  particles.emplace_back(p1);
  particles.emplace_back(p2);

  DistanceConstraint constraint(0, 1, 1.0, 0.0);
  constraint.solve(particles, 0.016);

  double distance =
      (particles[0].getPosition() - particles[1].getPosition()).norm();

  EXPECT_NEAR(1.0, distance, 1e-9);
}

TEST(DistanceConstraint, LowerComplianceConvergeFasterToRestLenght) {
  std::vector<Particle> particles;

  particles.emplace_back(Eigen::Vector3d(4.0, 4.0, 4.0));
  particles.emplace_back(Eigen::Vector3d(0.0, 0.0, 0.0));
  particles.emplace_back(Eigen::Vector3d(4.0, 4.0, 4.0));
  particles.emplace_back(Eigen::Vector3d(0.0, 0.0, 0.0));

  int iterations = 5;

  DistanceConstraint constraintBC(0, 1, 1.0, 0.8);
  for (int idx = 0; idx < iterations; idx++)
    constraintBC.solve(particles, 0.016);

  DistanceConstraint constraintLC(2, 3, 1.0, 0.2);
  for (int idx = 0; idx < iterations; idx++)
    constraintLC.solve(particles, 0.016);

  double distanceBC =
      (particles[0].getPosition() - particles[1].getPosition()).norm();
  double distanceLC =
      (particles[2].getPosition() - particles[3].getPosition()).norm();

  double errBC = 1.0 - distanceBC;
  double errLC = 1.0 - distanceLC;

  EXPECT_LT(errBC, errLC);
}