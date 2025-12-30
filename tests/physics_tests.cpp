#include <gtest/gtest.h>
#include "physics/Particle.hpp"
#include "physics/Solver.hpp"
#include <Eigen/Dense>

using namespace ClothSDK;

TEST(ParticleTest, Initialization) {
    Eigen::Vector3d pos(0, 10, 0);
    Particle p(pos);

    EXPECT_TRUE(p.getPosition().isApprox(pos));
    EXPECT_TRUE(p.getOldPosition().isApprox(pos));
    EXPECT_DOUBLE_EQ(p.getInverseMass(), 1.0);
}

TEST(SolverTest, GravityFall) {
    Solver solver;
    solver.setGravity(Eigen::Vector3d(0, -10, 0)); 
    
    Particle p(Eigen::Vector3d(0, 10, 0));
    solver.addParticle(p);

    solver.update(0.1); 

    const auto& particles = solver.getParticles();
    EXPECT_LT(particles[0].getPosition().y(), 10.0);
}

TEST(SolverTest, DistanceConstraintCheck) {
    Solver solver;
    solver.setGravity(Eigen::Vector3d(0, -10, 0));
    solver.setIterations(10); 

    Particle p0(Eigen::Vector3d(0, 0, 0));
    p0.setInverseMass(0.0); 
    
    Particle p1(Eigen::Vector3d(1, 0, 0)); 
    p1.setInverseMass(1.0);

    solver.addParticle(p0);
    solver.addParticle(p1);

    solver.addDistanceConstraint(0, 1, 1.0);

    for(int i = 0; i < 50; ++i) {
        solver.update(0.01);
    }

    const auto& particles = solver.getParticles();
    
    EXPECT_TRUE(particles[0].getPosition().isApprox(Eigen::Vector3d(0, 0, 0)));

    double finalDist = (particles[0].getPosition() - particles[1].getPosition()).norm();
    
    EXPECT_NEAR(finalDist, 1.0, 0.01);
}