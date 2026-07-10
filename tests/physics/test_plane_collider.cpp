#include "physics/Particle.hpp"
#include "physics/PlaneCollider.hpp"
#include "gtest/gtest.h"

using namespace Tissu;

TEST(PlaneCollider, ParticleUnderThePlaneMovesUpside) {
    Eigen::Vector3d origin(0.0, 0.0, 0.0);
    Eigen::Vector3d normal(0.0, 1.0, 0.0);
    PlaneCollider collider(origin, normal, 0.0);

    Eigen::Vector3d initialPos(0.0, -1.0, 0.0);

    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    collider.resolve(particles, 0.016, 0.0);

    EXPECT_GE(particles[0].getPosition().y(), origin.y());
}

TEST(PlaneCollider, ParticleUpsideDoesNotChangeItsPosition) {
    Eigen::Vector3d origin(0.0, 0.0, 0.0);
    Eigen::Vector3d normal(0.0, 1.0, 0.0);
    PlaneCollider collider(origin, normal, 0.0);

    Eigen::Vector3d initialPos(2.0, 8.0, 5.0);

    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    collider.resolve(particles, 0.016, 0.0);

    EXPECT_EQ(particles[0].getPosition().x(), initialPos.x());
    EXPECT_EQ(particles[0].getPosition().y(), initialPos.y());
    EXPECT_EQ(particles[0].getPosition().z(), initialPos.z());
}

TEST(PlaneCollider, ParticleInThicknessRadioChangesItsPosition) {
    Eigen::Vector3d origin(0.0, 0.0, 0.0);
    Eigen::Vector3d normal(0.0, 1.0, 0.0);
    PlaneCollider collider(origin, normal, 0.0);

    Eigen::Vector3d initialPos(2.0, 2.0, 2.0);

    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    collider.resolve(particles, 0.016, 3.0);

    Eigen::Vector3d predictedPosition;
    predictedPosition = initialPos + normal * (3.0 - 2.0);

    EXPECT_NEAR(particles[0].getPosition().x(), predictedPosition.x(), 1e-4);
    EXPECT_NEAR(particles[0].getPosition().y(), predictedPosition.y(), 1e-4);
    EXPECT_NEAR(particles[0].getPosition().z(), predictedPosition.z(), 1e-4);
}

TEST(PlaneCollider, FullFrictionCancelsTangentialVelocity) {
    Eigen::Vector3d origin(0.0, 0.0, 0.0);
    Eigen::Vector3d normal(0.0, 1.0, 0.0);
    PlaneCollider collider(origin, normal, 1.0);

    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(0.0, -0.5, 0.0));
    particles[0].setOldPosition(Eigen::Vector3d(-1.0, -0.5, 0.0));

    collider.resolve(particles, 0.016, 0.0);

    double velocityX =
        particles[0].getPosition().x() - particles[0].getOldPosition().x();
    double velocityZ =
        particles[0].getPosition().z() - particles[0].getOldPosition().z();

    EXPECT_NEAR(velocityX, 0.0, 1e-9);
    EXPECT_NEAR(velocityZ, 0.0, 1e-9);
}

TEST(PlaneCollider, NoFrictionDoesNotChangeTangentialVelocity) {
    Eigen::Vector3d origin(0.0, 0.0, 0.0);
    Eigen::Vector3d normal(0.0, 1.0, 0.0);
    PlaneCollider collider(origin, normal, 0.0);

    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(0.0, -0.5, 0.0));
    particles[0].setOldPosition(Eigen::Vector3d(-1.0, -0.5, 0.0));

    collider.resolve(particles, 0.016, 0.0);

    double velocityX =
        particles[0].getPosition().x() - particles[0].getOldPosition().x();

    EXPECT_NEAR(velocityX, 1, 1e-9);
}

TEST(PlaneCollider, TransformChangesPositionAndNormal) {
    Eigen::Vector3d origin(0.0, 0.0, 0.0);
    Eigen::Vector3d normal(0.0, 1.0, 0.0);
    PlaneCollider collider(origin, normal, 0.0);

    Eigen::Vector3d newPosition(1.0, 2.0, 3.0);
    Eigen::Quaterniond newRotation(
        Eigen::AngleAxisd(M_PI / 4, Eigen::Vector3d::UnitX()));

    collider.transform(newPosition, newRotation);

    EXPECT_EQ(collider.getOrigin(), newPosition);
    EXPECT_TRUE(collider.getNormal().isApprox(newRotation * normal));
}
