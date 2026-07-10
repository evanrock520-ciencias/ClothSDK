#include <vector>

#include "Eigen/Dense"
#include "physics/MeshCollider.hpp"
#include "physics/Particle.hpp"
#include "gtest/gtest.h"

using namespace Tissu;

static MeshCollider makeTetrahedron(double friction = 0.0) {
    std::vector<Eigen::Vector3d> vertices = {
        {0.0, 0.0, 0.0},
        {2.0, 0.0, 0.0},
        {1.0, 0.0, 2.0},
        {1.0, 2.0, 1.0},
    };
    std::vector<std::array<int, 3>> triangles = {
        {0, 2, 1},
        {0, 1, 3},
        {1, 2, 3},
        {0, 3, 2},
    };
    return MeshCollider(vertices, triangles, friction);
}

TEST(MeshCollider, ParticleInsideMeshMovesOutside) {
    MeshCollider mesh = makeTetrahedron(0.5);

    Eigen::Vector3d initialPos(1.0, 0.5, 0.75);
    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    mesh.resolve(particles, 0.016, 1.0);

    double distanceMoved = (particles[0].getPosition() - initialPos).norm();
    EXPECT_GT(distanceMoved, 0.0);
}

TEST(MeshCollider, ParticleOutsideMeshDoesNotChangePosition) {
    MeshCollider mesh = makeTetrahedron(0.0);

    Eigen::Vector3d initialPos(100.0, 100.0, 100.0);
    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    mesh.resolve(particles, 0.016, 0.0);

    EXPECT_EQ(particles[0].getPosition(), initialPos);
}

TEST(MeshCollider, ParticleInThicknessRangeIsProjectedOutside) {
    MeshCollider mesh = makeTetrahedron(0.0);

    Eigen::Vector3d initialPos(1.0, 0.5, 0.75);
    std::vector<Particle> particles;
    particles.emplace_back(initialPos);

    mesh.resolve(particles, 0.016, 2.0);

    double distance = (particles[0].getPosition() - initialPos).norm();
    EXPECT_GT(distance, 0.0);
}

TEST(MeshCollider, MultipleParticlesAreAllResolved) {
    MeshCollider mesh = makeTetrahedron(0.0);

    std::vector<Particle> particles;
    particles.emplace_back(Eigen::Vector3d(1.0, 0.5, 0.75));
    particles.emplace_back(Eigen::Vector3d(100.0, 100.0, 100.0));
    particles.emplace_back(Eigen::Vector3d(0.5, 0.25, 0.375));
    particles.emplace_back(Eigen::Vector3d(1.0, 1.0, 1.0));

    mesh.resolve(particles, 0.016, 1.0);

    EXPECT_NE(particles[0].getPosition(), particles[0].getOldPosition());
    EXPECT_EQ(particles[1].getPosition(), particles[1].getOldPosition());
    EXPECT_NE(particles[2].getPosition(), particles[2].getOldPosition());
}

TEST(MeshCollider, FullFrictionCancelsTangentialVelocity) {
    MeshCollider mesh = makeTetrahedron(1.0);

    Eigen::Vector3d pos(1.0, -0.01, 0.75);
    std::vector<Particle> particles;
    particles.emplace_back(pos);
    particles[0].setOldPosition(Eigen::Vector3d(0.0, -0.01, 0.75));

    mesh.resolve(particles, 0.016, 0.1);

    double velocityX =
        particles[0].getPosition().x() - particles[0].getOldPosition().x();
    double velocityZ =
        particles[0].getPosition().z() - particles[0].getOldPosition().z();

    EXPECT_NEAR(velocityX, 0.0, 1e-9);
    EXPECT_NEAR(velocityZ, 0.0, 1e-9);
}

TEST(MeshCollider, NoFrictionDoesNotChangeTangentialVelocity) {
    MeshCollider mesh = makeTetrahedron(0.0);

    Eigen::Vector3d pos(1.0, -0.01, 0.75);
    std::vector<Particle> particles;
    particles.emplace_back(pos);
    particles[0].setOldPosition(Eigen::Vector3d(0.0, -0.01, 0.75));

    mesh.resolve(particles, 0.016, 0.1);

    double velocityX =
        particles[0].getPosition().x() - particles[0].getOldPosition().x();

    EXPECT_NEAR(velocityX, 1.0, 1e-6);
}

TEST(MeshCollider, TransformMovesMeshAndAffectsResolution) {
    MeshCollider mesh = makeTetrahedron(0.0);

    Eigen::Vector3d insidePos(1.0, 0.5, 0.75);

    std::vector<Particle> beforeParticles;
    beforeParticles.emplace_back(insidePos);
    mesh.resolve(beforeParticles, 0.016, 1.0);
    bool movedBefore =
        (beforeParticles[0].getPosition() - insidePos).norm() > 0.0;

    MeshCollider transformed = makeTetrahedron(0.0);
    transformed.transform(Eigen::Vector3d(10.0, 10.0, 10.0),
                          Eigen::Quaterniond::Identity());

    std::vector<Particle> afterParticles;
    afterParticles.emplace_back(insidePos);
    transformed.resolve(afterParticles, 0.016, 1.0);
    bool movedAfter =
        (afterParticles[0].getPosition() - insidePos).norm() > 0.0;

    EXPECT_TRUE(movedBefore);
    EXPECT_FALSE(movedAfter);
}

TEST(MeshCollider, TransformWithRotationWorks) {
    MeshCollider mesh = makeTetrahedron(0.0);

    Eigen::Vector3d insidePos(1.0, 0.5, 0.75);
    std::vector<Particle> beforeParticles;
    beforeParticles.emplace_back(insidePos);
    mesh.resolve(beforeParticles, 0.016, 1.0);
    bool movedBefore =
        (beforeParticles[0].getPosition() - insidePos).norm() > 0.0;

    MeshCollider rotated = makeTetrahedron(0.0);
    Eigen::Quaterniond rot(Eigen::AngleAxisd(M_PI, Eigen::Vector3d::UnitY()));
    rotated.transform(Eigen::Vector3d(0.0, 0.0, 0.0), rot);

    EXPECT_TRUE(movedBefore);

    std::vector<Particle> rotatedParticles;
    rotatedParticles.emplace_back(Eigen::Vector3d(-1.0, 0.5, -0.75));
    rotated.resolve(rotatedParticles, 0.016, 1.0);
    EXPECT_NE(rotatedParticles[0].getPosition(),
              rotatedParticles[0].getOldPosition());
}