#include "physics/Particle.hpp"
#include <gtest/gtest.h>

using namespace Tissu;

TEST(Particle, InitialPosition) {
  Eigen::Vector3d expectedPos(1.0, 2.0, 3.0);

  Particle p(expectedPos);

  EXPECT_NEAR(p.getPosition().x(), expectedPos.x(), 1e-9);
  EXPECT_NEAR(p.getPosition().y(), expectedPos.y(), 1e-9);
  EXPECT_NEAR(p.getPosition().z(), expectedPos.z(), 1e-9);
}

TEST(Particle, SetPositionUpdateCurrentPosition) {
  Eigen::Vector3d initalPos(1.0, 2.0, 3.0);
  Eigen::Vector3d expectedPos(4.0, 5.0, 6.0);

  Particle p(initalPos);
  p.setPosition(expectedPos);

  EXPECT_NEAR(p.getPosition().x(), expectedPos.x(), 1e-9);
  EXPECT_NEAR(p.getPosition().y(), expectedPos.y(), 1e-9);
  EXPECT_NEAR(p.getPosition().z(), expectedPos.z(), 1e-9);
}

TEST(Particle, SetPositionPreserveOldPosition) {
  Eigen::Vector3d initalPos(1.0, 2.0, 3.0);
  Eigen::Vector3d expectedPos(4.0, 5.0, 6.0);

  Particle p(initalPos);
  p.setPosition(expectedPos);

  EXPECT_NEAR(p.getOldPosition().x(), initalPos.x(), 1e-9);
  EXPECT_NEAR(p.getOldPosition().y(), initalPos.y(), 1e-9);
  EXPECT_NEAR(p.getOldPosition().z(), initalPos.z(), 1e-9);
}

TEST(Particle, StaticParticleDoesNotChangePositionAfterIntegrate) {
  Eigen::Vector3d expectedPos(1.0, 2.0, 3.0);

  Particle p(expectedPos);
  p.setInverseMass(0.0);
  p.integrate(0.016);

  EXPECT_NEAR(p.getPosition().x(), expectedPos.x(), 1e-9);
  EXPECT_NEAR(p.getPosition().y(), expectedPos.y(), 1e-9);
  EXPECT_NEAR(p.getPosition().z(), expectedPos.z(), 1e-9);
}

TEST(Particle, StaticParticleDoesNotChangePositionAfterApplyForces) {
  Eigen::Vector3d expectedPos(1.0, 2.0, 3.0);
  Eigen::Vector3d force(10.0, 2.0, 5.0);

  Particle p(expectedPos);
  p.setInverseMass(0.0);
  p.addForce(force);
  p.integrate(0.016);

  EXPECT_NEAR(p.getPosition().x(), expectedPos.x(), 1e-9);
  EXPECT_NEAR(p.getPosition().y(), expectedPos.y(), 1e-9);
  EXPECT_NEAR(p.getPosition().z(), expectedPos.z(), 1e-9);
}

TEST(Particle, AddMassUpdatesInverseMass) {
  Eigen::Vector3d expectedPos(1.0, 2.0, 3.0);

  Particle p(expectedPos);
  p.setInverseMass(1.0);
  p.addMass(1.0);

  EXPECT_EQ(0.5, p.getInverseMass());
}

TEST(Particle, StaticParticlePreserveZeroMassAfterAddMass) {
  Eigen::Vector3d expectedPos(1.0, 2.0, 3.0);

  Particle p(expectedPos);
  p.setInverseMass(0.0);
  p.addMass(10.0);

  EXPECT_EQ(0.0, p.getInverseMass());
}

TEST(Particle, GravityAffectsParticlePosition) {
  Eigen::Vector3d initialPos(2.0, 20.0, 8.0);
  Eigen::Vector3d gravity(0.0, -9.81, 0.0);

  int iterations = 10;

  Particle p(initialPos);
  p.setInverseMass(1.0);
  for (int idx = 0; idx < iterations; idx++) {
    p.addForce(gravity);
    p.integrate(0.016);
  }

  EXPECT_LT(p.getPosition().y(), initialPos.y());
}

TEST(Particle, SecondIntegrationHasBiggerVelocityThanTheFirstOne) {
  Eigen::Vector3d initialPos(2.0, 20.0, 8.0);
  Eigen::Vector3d gravity(0.0, -9.81, 0.0);

  Particle p(initialPos);
  // First Integration
  p.addForce(gravity);
  p.integrate(0.016);
  Eigen::Vector3d fiVel = p.getVelocity(0.016);

  // Second Integration
  p.addForce(gravity);
  p.integrate(0.016);
  Eigen::Vector3d siVel = p.getVelocity(0.016);

  EXPECT_LT(siVel.y(), fiVel.y());
}

TEST(Particle, ClearForcesWorks) {
  Eigen::Vector3d initialPos(2.0, 20.0, 8.0);
  Eigen::Vector3d gravity(0.0, -9.81, 0.0);

  Particle p(initialPos);
  p.addForce(gravity);
  p.clearForces();

  p.integrate(0.016);

  EXPECT_EQ(p.getPosition().x(), p.getOldPosition().x());
  EXPECT_EQ(p.getPosition().y(), p.getOldPosition().y());
  EXPECT_EQ(p.getPosition().z(), p.getOldPosition().z());
}