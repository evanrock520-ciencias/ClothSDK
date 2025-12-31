#include <gtest/gtest.h>
#include "physics/Particle.hpp"
#include <Eigen/Dense>

using namespace ClothSDK;

#define EXPECT_VECTOR3D_NEAR(v1, v2, tol) \
    EXPECT_NEAR(v1.x(), v2.x(), tol); \
    EXPECT_NEAR(v1.y(), v2.y(), tol); \
    EXPECT_NEAR(v1.z(), v2.z(), tol);

TEST(ParticleTest, Initialization) {
    Eigen::Vector3d pos(1.0, 2.0, 3.0);
    Particle p(pos);

    EXPECT_VECTOR3D_NEAR(p.getPosition(), pos, 1e-9);
    EXPECT_VECTOR3D_NEAR(p.getOldPosition(), pos, 1e-9);
    EXPECT_DOUBLE_EQ(p.getInverseMass(), 1.0);
}

TEST(ParticleTest, AddForce) {
    Particle p(Eigen::Vector3d::Zero());
    p.setInverseMass(0.5); 
    
    p.addForce(Eigen::Vector3d(10.0, 0.0, 0.0));
    EXPECT_VECTOR3D_NEAR(p.getAcceleration(), Eigen::Vector3d(5.0, 0.0, 0.0), 1e-9);
    
    p.clearForces();
    EXPECT_VECTOR3D_NEAR(p.getAcceleration(), Eigen::Vector3d::Zero(), 1e-9);
}

TEST(ParticleTest, IntegrationMovement) {
    Particle p(Eigen::Vector3d::Zero());
    double dt = 0.1;
    
    p.addForce(Eigen::Vector3d(10.0, 0.0, 0.0));
    
    p.integrate(dt);
    
    EXPECT_NEAR(p.getPosition().x(), 0.1, 1e-9);
    EXPECT_NEAR(p.getOldPosition().x(), 0.0, 1e-9); 
}

TEST(ParticleTest, StaticParticle) {
    Particle p(Eigen::Vector3d(1.0, 1.0, 1.0));
    p.setInverseMass(0.0); 
    
    p.addForce(Eigen::Vector3d(0.0, -9.8, 0.0));
    p.integrate(0.1);
    
    EXPECT_VECTOR3D_NEAR(p.getPosition(), Eigen::Vector3d(1.0, 1.0, 1.0), 1e-9);
}