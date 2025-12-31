#include <gtest/gtest.h>
#include "physics/Solver.hpp"
#include <Eigen/Dense>

using namespace ClothSDK;

TEST(SolverTest, GravityAndSubsteps) {
    Solver solver;
    solver.setGravity(Eigen::Vector3d(0, -10, 0));
    solver.setSubsteps(5); 
    
    int pId = solver.addParticle(Particle(Eigen::Vector3d(0, 10, 0)));
    
    solver.update(1.0);
    
    const auto& particles = solver.getParticles();
    EXPECT_NEAR(particles[pId].getPosition().y(), 4.0, 1e-3);
}

TEST(SolverTest, PlaneCollision) {
    Solver solver;
    solver.setGravity(Eigen::Vector3d(0, -10, 0));
    solver.addPlaneCollider(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 1, 0), 0.0);
    
    int pId = solver.addParticle(Particle(Eigen::Vector3d(0, 0.5, 0)));
    
    for(int i = 0; i < 10; ++i) solver.update(0.1);
    
    EXPECT_GE(solver.getParticles()[pId].getPosition().y(), -1e-6);
}

TEST(SolverTest, AerodynamicsAppliesForce) {
    Solver solver;
    solver.setGravity(Eigen::Vector3d::Zero()); 
    
    int a = solver.addParticle(Particle(Eigen::Vector3d(0, 0, 0)));
    int b = solver.addParticle(Particle(Eigen::Vector3d(1, 0, 0)));
    int c = solver.addParticle(Particle(Eigen::Vector3d(0, 1, 0)));
    solver.addAeroFace(a, b, c);
    
    solver.update(0.01);
    
    EXPECT_GT(std::abs(solver.getParticles()[a].getPosition().z()), 0.0);
}

TEST(SolverTest, ClearSystem) {
    Solver solver;
    solver.addParticle(Particle(Eigen::Vector3d::Zero()));
    solver.addPlaneCollider(Eigen::Vector3d::Zero(), Eigen::Vector3d::UnitY(), 0.0);
    
    solver.clear();
    
    EXPECT_EQ(solver.getParticles().size(), 0);
}