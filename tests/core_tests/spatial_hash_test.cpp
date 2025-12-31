#include <gtest/gtest.h>
#include "physics/SpatialHash.hpp"
#include "physics/Particle.hpp"
#include <vector>

using namespace ClothSDK;

class SpatialHashTest : public ::testing::Test {
protected:
    SpatialHash hash = SpatialHash(1000, 1.0);
    std::vector<Particle> particles;
};

TEST_F(SpatialHashTest, FindsNeighborInSameCell) {
    particles.push_back(Particle(Eigen::Vector3d(0.0, 0.0, 0.0)));
    particles.push_back(Particle(Eigen::Vector3d(0.1, 0.0, 0.0)));

    hash.build(particles);

    std::vector<int> neighbors;
    hash.query(particles, particles[0].getPosition(), 0.2, neighbors);

    EXPECT_EQ(neighbors.size(), 2);
}

TEST_F(SpatialHashTest, FindsNeighborInAdjacentCell) {
    particles.push_back(Particle(Eigen::Vector3d(0.9, 0.0, 0.0))); 
    particles.push_back(Particle(Eigen::Vector3d(1.1, 0.0, 0.0))); 

    hash.build(particles);

    std::vector<int> neighbors;
    hash.query(particles, particles[0].getPosition(), 0.5, neighbors);

    EXPECT_EQ(neighbors.size(), 2);
}

TEST_F(SpatialHashTest, FiltersOutParticlesBeyondRadius) {
    particles.push_back(Particle(Eigen::Vector3d(0.0, 0.0, 0.0)));
    particles.push_back(Particle(Eigen::Vector3d(0.9, 0.0, 0.0)));

    hash.build(particles);

    std::vector<int> neighbors;
    hash.query(particles, particles[0].getPosition(), 0.5, neighbors);

    EXPECT_EQ(neighbors.size(), 1);
    EXPECT_EQ(neighbors[0], 0);
}

TEST_F(SpatialHashTest, HandlesMultipleParticles) {
    for(int i = 0; i < 10; ++i) {
        particles.push_back(Particle(Eigen::Vector3d(i * 0.1, 0.0, 0.0)));
    }

    hash.build(particles);

    std::vector<int> neighbors;
    hash.query(particles, particles[5].getPosition(), 0.15, neighbors);

    EXPECT_EQ(neighbors.size(), 3);
}