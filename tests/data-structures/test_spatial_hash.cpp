#include "data-structures/SpatialHash.hpp"
#include "physics/Particle.hpp"
#include "gtest/gtest.h"
#include <vector>

using namespace Tissu;

TEST(SpatialHash, QueryOnParticlePositionFindsTheParticleOnThatPosition) {
  std::vector<Particle> particles;
  Eigen::Vector3d pos(1.0, 0.0, 0.0);

  particles.emplace_back(pos);

  SpatialHash hash(1000, 1.0);
  hash.build(particles);

  std::vector<int> neighbors;
  hash.query(particles, pos, 1.0, neighbors);

  EXPECT_EQ(1, neighbors.size());
}

TEST(SpatialHash, QueryDoesNotFindOutOfRadioParticles) {
  std::vector<Particle> particles;
  Eigen::Vector3d queryPos(0.0, 1.0, 0.0);
  Eigen::Vector3d posA(10.0, 0.0, 0.0);
  Eigen::Vector3d posB(0.0, 100.0, 0.0);
  Eigen::Vector3d posC(10.0, 90.0, 1.0);

  particles.emplace_back(posA);
  particles.emplace_back(posB);
  particles.emplace_back(posC);

  SpatialHash hash(1000, 1.0);
  hash.build(particles);

  std::vector<int> neighbors;
  hash.query(particles, queryPos, 2.0, neighbors);

  EXPECT_EQ(0, neighbors.size());
}

TEST(SpatialHash, QueryFindsAllParticlesInRadio) {
  std::vector<Particle> particles;
  Eigen::Vector3d queryPos(0.0, 1.0, 0.0);
  Eigen::Vector3d posA(1.0, 0.0, 0.0);
  Eigen::Vector3d posB(0.0, 1.0, 0.0);
  Eigen::Vector3d posC(0.0, 0.0, 1.0);

  particles.emplace_back(posA);
  particles.emplace_back(posB);
  particles.emplace_back(posC);

  SpatialHash hash(1000, 1.0);
  hash.build(particles);

  std::vector<int> neighbors;
  hash.query(particles, queryPos, 2.0, neighbors);

  EXPECT_EQ(particles.size(), neighbors.size());
}

TEST(SpatialHash, BuildWithEmptyVectorMakesQueryReturnAnEmptyVector) {
  std::vector<Particle> particles;
  Eigen::Vector3d queryPos(0.0, 1.0, 0.0);

  SpatialHash hash(1000, 1.0);
  hash.build(particles);

  std::vector<int> neighbors;
  hash.query(particles, queryPos, 1.0, neighbors);

  EXPECT_EQ(0, neighbors.size());
}

TEST(SpatialHash, ParticleWithBorderPositionDoesNotAppearInQuery) {
  std::vector<Particle> particles;
  Eigen::Vector3d queryPos(0.0, 0.0, 0.0);
  Eigen::Vector3d particlePos(0.0, 1.0, 0.0);

  particles.emplace_back(particlePos);

  SpatialHash hash(1000, 1.0);
  hash.build(particles);

  std::vector<int> neighbors;
  hash.query(particles, queryPos, 1.0, neighbors);

  EXPECT_EQ(0, neighbors.size());
}