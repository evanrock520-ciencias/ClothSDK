#include <gtest/gtest.h>
#include "physics/BendingConstraint.hpp"
#include "physics/Particle.hpp"
#include <vector>
#include <cmath>

using namespace ClothSDK;

double calculateAngle(const Eigen::Vector3d& pA, const Eigen::Vector3d& pB, 
                      const Eigen::Vector3d& pC, const Eigen::Vector3d& pD) {
    Eigen::Vector3d edge = pB - pA;
    Eigen::Vector3d n1 = edge.cross(pC - pA);
    Eigen::Vector3d n2 = edge.cross(pD - pA);
    return std::acos(std::clamp(n1.dot(n2) / (n1.norm() * n2.norm()), -1.0, 1.0));
}

TEST(BendingConstraintTest, NoMovementAtRest) {
    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(0, 0, 0));
    particles.emplace_back(Eigen::Vector3d(0, 0, 1));
    particles.emplace_back(Eigen::Vector3d(1, 0, 0.5));
    particles.emplace_back(Eigen::Vector3d(-1, 0, 0.5));

    double currentAngle = calculateAngle(particles[0].getPosition(), particles[1].getPosition(),
                                         particles[2].getPosition(), particles[3].getPosition());

    Eigen::Vector3d oldPosC = particles[2].getPosition();
    
    BendingConstraint constraint(0, 1, 2, 3, currentAngle, 0.0);
    constraint.solve(particles, 0.01);

    EXPECT_NEAR((particles[2].getPosition() - oldPosC).norm(), 0.0, 1e-6);
}