#include <vector>

#include "gtest/gtest.h"
#include "physics/Particle.hpp"
#include "physics/SphereCollider.hpp"

using namespace Tissu;

TEST(SphereCollider, ParticleInsideTheSphereMovesOutside) {
  Eigen::Vector3d center(0.0, 0.0, 0.0);
  double radius = 5.0;
  SphereCollider sphere(center, radius, 0.0);

  Eigen::Vector3d initialPos(0.0, 1.0, 0.0);
  std::vector<Particle> particles;
  particles.emplace_back(initialPos);

  sphere.resolve(particles, 0.016, 0.0);

  double distance = (particles[0].getPosition() - center).norm();

  EXPECT_NEAR(distance, radius, 1e-6);
}

TEST(SphereCollider, ParticleOutsideTheSphereDoesNotChangeItsPosition) {
  Eigen::Vector3d center(0.0, 0.0, 0.0);
  double radius = 5.0;
  SphereCollider sphere(center, radius, 0.0);

  Eigen::Vector3d initialPos(8.0, 10.0, 0.0);
  std::vector<Particle> particles;
  particles.emplace_back(initialPos);

  sphere.resolve(particles, 0.016, 0.0);

  EXPECT_EQ(particles[0].getPosition().x(), initialPos.x());
  EXPECT_EQ(particles[0].getPosition().y(), initialPos.y());
  EXPECT_EQ(particles[0].getPosition().z(), initialPos.z());
}

TEST(SphereCollider, ParticleInThicknessRangeIsProjectedToSurface) {
  Eigen::Vector3d center(0.0, 0.0, 0.0);
  double radius = 5.0;
  double thickness = 3.0;
  SphereCollider sphere(center, radius, 0.0);

  std::vector<Particle> particles;
  particles.emplace_back(Eigen::Vector3d(0.0, 6.0, 0.0));

  sphere.resolve(particles, 0.016, thickness);

  double distance = (particles[0].getPosition() - center).norm();
  EXPECT_NEAR(distance, radius + thickness, 1e-6);
}

TEST(SphereCollider, FullFrictionCancelsTangentialVelocity) {
  Eigen::Vector3d origin(0.0, 0.0, 0.0);
  double radius = 2.0;
  SphereCollider sphere(origin, radius, 1.0);

  std::vector<Particle> particles;
  particles.emplace_back(Eigen::Vector3d(0.0, -0.5, 0.0));
  particles[0].setOldPosition(Eigen::Vector3d(-1.0, -0.5, 0.0));

  sphere.resolve(particles, 0.016, 0.0);

  double velocityX =
      particles[0].getPosition().x() - particles[0].getOldPosition().x();
  double velocityZ =
      particles[0].getPosition().z() - particles[0].getOldPosition().z();

  EXPECT_NEAR(velocityX, 0.0, 1e-9);
  EXPECT_NEAR(velocityZ, 0.0, 1e-9);
}

TEST(SphereCollider, NoFrictionDoesNotChangeTangentialVelocity) {
  Eigen::Vector3d origin(0.0, 0.0, 0.0);
  double radius = 2.0;
  SphereCollider sphere(origin, radius, 0.0);

  std::vector<Particle> particles;
  particles.emplace_back(Eigen::Vector3d(0.0, -0.5, 0.0));
  particles[0].setOldPosition(Eigen::Vector3d(-1.0, -0.5, 0.0));

  sphere.resolve(particles, 0.016, 0.0);

  double velocityX =
      particles[0].getPosition().x() - particles[0].getOldPosition().x();

  EXPECT_NEAR(velocityX, 1, 1e-9);
}

TEST(SphereCollider, ParticleOnSphereCenterProjectsUpward) {
  Eigen::Vector3d center(0.0, 0.0, 0.0);
  double radius = 5.0;
  SphereCollider sphere(center, radius, 0.0);

  std::vector<Particle> particles;
  particles.emplace_back(center);

  sphere.resolve(particles, 0.016, 0.1);

  EXPECT_NEAR(particles[0].getPosition().x(), 0.0, 1e-6);
  EXPECT_GT(particles[0].getPosition().y(), 0.0);
  double distance = (particles[0].getPosition() - center).norm();
  EXPECT_NEAR(distance, radius + 0.1, 1e-6);
}