#include <gtest/gtest.h>
#include "physics/DistanceConstraint.hpp"
#include "physics/Particle.hpp"
#include <vector>

using namespace ClothSDK;

TEST(DistanceConstraintTest, SolveBasicStiffness) {
    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(0.0, 0.0, 0.0));
    particles.emplace_back(Eigen::Vector3d(2.0, 0.0, 0.0));
    
    double restLength = 1.0;
    double compliance = 0.0;
    DistanceConstraint constraint(0, 1, restLength, compliance);
    
    double dt = 0.01;
    
    constraint.solve(particles, dt);
    
    double finalDist = (particles[0].getPosition() - particles[1].getPosition()).norm();
    
    EXPECT_NEAR(finalDist, 1.0, 1e-6);
}

TEST(DistanceConstraintTest, StaticParticleImmunity) {
    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(0.0, 0.0, 0.0));
    particles.emplace_back(Eigen::Vector3d(2.0, 0.0, 0.0));
    
    particles[0].setInverseMass(0.0); 
    
    DistanceConstraint constraint(0, 1, 1.0, 0.0);
    constraint.solve(particles, 0.01);
    
    EXPECT_DOUBLE_EQ(particles[0].getPosition().x(), 0.0);
    EXPECT_DOUBLE_EQ(particles[1].getPosition().x(), 1.0);
}