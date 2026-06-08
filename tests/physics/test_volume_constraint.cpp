#include <string>

#include "gtest/gtest.h"
#include "physics/VolumeConstraint.hpp"
#include "utils/Logger.hpp"

using namespace Tissu;

class VolumeConstraintTest : public ::testing::Test {
 protected:
  class TestableVolumeConstraint : public VolumeConstraint {
   public:
    using VolumeConstraint::computeVolume;
    using VolumeConstraint::VolumeConstraint;
  };

  void SetUp() override {
    particles.emplace_back(Eigen::Vector3d(1.0, 0.0, -0.707));
    particles.emplace_back(Eigen::Vector3d(-1.0, 0.0, -0.707));
    particles.emplace_back(Eigen::Vector3d(0.0, 1.0, 0.707));
    particles.emplace_back(Eigen::Vector3d(0.0, -1.0, 0.707));

    triangles.push_back({0, 1, 2});
    triangles.push_back({0, 2, 3});
    triangles.push_back({0, 3, 1});
    triangles.push_back({1, 3, 2});
  }

  std::vector<Particle> particles;
  std::vector<Triangle> triangles;
};

TEST_F(VolumeConstraintTest, ConvergesToRestVolume) {
  TestableVolumeConstraint constraint(triangles, particles, 0.0);
  double restVolume = constraint.getRestVolume();

  for (int idx = 0; idx < 100; idx++) constraint.solve(particles, 0.016);

  double currentVolume = constraint.computeVolume(particles);
  EXPECT_NEAR(currentVolume, restVolume, 1e-4);
}

TEST_F(VolumeConstraintTest, CompressionPushesOutward) {
  TestableVolumeConstraint constraint(triangles, particles, 0.0);

  for (auto& particle : particles) {
    particle.setPosition(particle.getPosition() * 0.5);
  }

  double compressedVolume = constraint.computeVolume(particles);

  int iterations = 100;
  for (int idx = 0; idx < iterations; idx++) constraint.solve(particles, 0.016);

  double finalVolume = constraint.computeVolume(particles);
  EXPECT_GT(finalVolume, compressedVolume);
}

TEST_F(VolumeConstraintTest, StaticParticlesDoNotMove) {
  TestableVolumeConstraint constraint(triangles, particles, 0.0);
  particles[0].setInverseMass(0.0);

  int iterations = 100;
  for (int idx = 0; idx < iterations; idx++) constraint.solve(particles, 0.016);

  EXPECT_EQ(particles[0].getPosition().x(), 1);
  EXPECT_EQ(particles[0].getPosition().y(), 0);
  EXPECT_NEAR(particles[0].getPosition().z(), -0.707, 1e-4);
}

TEST_F(VolumeConstraintTest, LowerComplianceConvergesFaster) {
  std::vector<Particle> particlesA = particles;
  std::vector<Particle> particlesB = particles;

  TestableVolumeConstraint constraintA(triangles, particlesA, 0.2);
  TestableVolumeConstraint constraintB(triangles, particlesB, 0.8);

  for (auto& p : particlesA) p.setPosition(p.getPosition() * 0.5);
  for (auto& p : particlesB) p.setPosition(p.getPosition() * 0.5);

  int iterations = 5;
  for (int idx = 0; idx < iterations; idx++) {
    constraintA.solve(particlesA, 0.016);
    constraintB.solve(particlesB, 0.016);
  }

  double volA = constraintA.computeVolume(particlesA);
  double volB = constraintB.computeVolume(particlesB);
  double restVolume = constraintA.getRestVolume();

  double errorA = std::abs(volA - restVolume);
  double errorB = std::abs(volB - restVolume);

  EXPECT_LT(errorA, errorB);
}