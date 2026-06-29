#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"
#include "io/StateSerializer.hpp"
#include "math/Types.hpp"
#include "physics/Constraint.hpp"
#include "physics/Particle.hpp"
#include "physics/Solver.hpp"

#include "physics/PlaneCollider.hpp"
#include "physics/SphereCollider.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/MeshCollider.hpp"

using namespace Tissu;
namespace fs = std::filesystem;

class StateSerializerTest : public ::testing::Test {
 protected:
  void SetUp() override {
    m_tempFile = fs::temp_directory_path() / "state.tissu";
  }

  void TearDown() override {
    if (fs::exists(m_tempFile)) fs::remove(m_tempFile);
  }

  fs::path m_tempFile;
};

TEST_F(StateSerializerTest, SaveThrowsIfPathIsInvalid) {
  Solver solver;
  World world;
  EXPECT_THROW(
      StateSerializer::save("/nonexistent/path/state.tissu", solver, world),
      std::runtime_error);
}

TEST_F(StateSerializerTest, LoadThrowsIfFileNotFound) {
  Solver solver;
  World world;
  EXPECT_THROW(StateSerializer::load("nonexistent.tissu", solver, world),
               std::runtime_error);
}

TEST_F(StateSerializerTest, FileIsTooSmall) {
  std::ofstream file(m_tempFile, std::ios::binary);
  file << "abc";
  file.close();

  Solver solver;
  World world;
  EXPECT_FALSE(StateSerializer::load(m_tempFile.string(), solver, world));
}

TEST_F(StateSerializerTest, InvalidMagicBytes) {
  Solver solver;
  World world;
  StateSerializer::save(m_tempFile.string(), solver, world);

  std::fstream f(m_tempFile, std::ios::binary | std::ios::in | std::ios::out);
  f.seekp(0);
  f.write("XXXXX", 5);
  f.close();

  Solver loadedSolver;
  World loadedWorld;

  EXPECT_FALSE(
      StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld));
}

TEST_F(StateSerializerTest, CrcMismatch) {
  Solver solver;
  World world;
  StateSerializer::save(m_tempFile.string(), solver, world);

  std::fstream f(m_tempFile, std::ios::binary | std::ios::in | std::ios::out);
  f.seekp(40);
  char corrupted = 0xFF;
  f.write(&corrupted, 1);
  f.close();

  Solver loadedSolver;
  World loadedWorld;
  EXPECT_FALSE(
      StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld));
}

TEST_F(StateSerializerTest, RoundTripWorldParameters) {
  Solver solver;
  World world;

  world.setAirDensity(0.02);
  world.setThickness(0.05);
  world.setGravity(Eigen::Vector3d(0.0, -9.81, 0.0));
  world.setWind(Eigen::Vector3d(1.5, 6.6, 0.2));

  StateSerializer::save(m_tempFile.string(), solver, world);

  Solver loadedSolver;
  World loadedWorld;

  StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld);

  EXPECT_NEAR(world.getAirDensity(), loadedWorld.getAirDensity(), 1e-9);
  EXPECT_NEAR(world.getThickness(), loadedWorld.getThickness(), 1e-9);
  EXPECT_TRUE(world.getGravity().isApprox(loadedWorld.getGravity()));  // Trying
  EXPECT_TRUE(world.getWind().isApprox(loadedWorld.getWind()));
}

TEST_F(StateSerializerTest, RoundTripParticleState) {
  Solver solver;
  World world;

  Particle p(Eigen::Vector3d(1.0, 2.0, 3.0));
  p.setOldPosition(Eigen::Vector3d(0.9, 1.9, 2.9));
  p.setInverseMass(2.0);
  solver.addParticle(p);

  StateSerializer::save(m_tempFile.string(), solver, world);

  Solver loadedSolver;
  World loadedWorld;

  loadedSolver.addParticle(Eigen::Vector3d(0.0, 0.0, 0.0));

  StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld);

  const auto& loaded = loadedSolver.getParticles();
  const auto& original = solver.getParticles();
  EXPECT_TRUE(loaded[0].getPosition().isApprox(original[0].getPosition()));
  EXPECT_TRUE(
      loaded[0].getOldPosition().isApprox(original[0].getOldPosition()));
  EXPECT_NEAR(loaded[0].getInverseMass(), original[0].getInverseMass(), 1e-9);
}

TEST_F(StateSerializerTest, RoundTripParticles) {
  Solver solver;
  World world;
  auto material = std::make_shared<ClothMaterial>();
  auto cloth = std::make_shared<Cloth>("test", material);

  ClothMesh mesh;
  mesh.initGrid(50, 50, 0.05, *cloth, solver);

  for (size_t idx = 0; idx < 30; idx++) solver.update(world, 1.0 / 60.0);

  std::vector<Particle> initial = solver.getParticles();
  StateSerializer::save(m_tempFile.string(), solver, world);

  Solver loadedSolver;
  World loadedWorld;
  auto loadedMaterial = std::make_shared<ClothMaterial>();
  auto loadedCloth = std::make_shared<Cloth>("test", material);
  mesh.initGrid(50, 50, 50, *loadedCloth, loadedSolver);
  StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld);

  std::vector<Particle> loaded = loadedSolver.getParticles();

  for (size_t idx = 0; idx < solver.getParticleCount(); idx++) {
    EXPECT_TRUE(initial[idx].getPosition().isApprox(loaded[idx].getPosition()));
    EXPECT_TRUE(
        initial[idx].getOldPosition().isApprox(loaded[idx].getOldPosition()));
    EXPECT_NEAR(initial[idx].getInverseMass(), loaded[idx].getInverseMass(),
                1e-9);
  }
}

TEST_F(StateSerializerTest, RoundTripConstraints) {
  Solver solver;
  World world;
  auto material = std::make_shared<ClothMaterial>();
  auto cloth = std::make_shared<Cloth>("test", material);

  ClothMesh mesh;
  mesh.initGrid(50, 50, 0.05, *cloth, solver);

  for (size_t idx = 0; idx < 30; idx++) solver.update(world, 1.0 / 60.0);

  const auto& initial = solver.getConstraints();
  StateSerializer::save(m_tempFile.string(), solver, world);

  Solver loadedSolver;
  World loadedWorld;
  auto loadedMaterial = std::make_shared<ClothMaterial>();
  auto loadedCloth = std::make_shared<Cloth>("test", material);
  mesh.initGrid(50, 50, 50, *loadedCloth, loadedSolver);
  StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld);

  const auto& loaded = loadedSolver.getConstraints();

  for (size_t idx = 0; idx < loaded.size(); idx++)
    EXPECT_NEAR(initial[idx]->getLambda(), loaded[idx]->getLambda(), 1e-9);
}

TEST_F(StateSerializerTest, RoundTripColliders) {
  Solver solver;
  World world;

  auto plane = std::make_shared<PlaneCollider>(Eigen::Vector3d(0.0, 0.0, 0.0),
                                               Eigen::Vector3d(0.0, 1.0, 0.0), 0.5);
  auto sphere = std::make_shared<SphereCollider>(Eigen::Vector3d(1.0, 1.0, 1.0),
                                                 0.5, 0.3);
  auto capsule = std::make_shared<CapsuleCollider>(0.25, Eigen::Vector3d(2.0, 2.0, 2.0), Eigen::Vector3d(3.0, 3.0, 3.0), 0.2);

  std::vector<Eigen::Vector3d> vertices = {
      {0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}};
  std::vector<std::array<int, 3>> triangles = {{{0, 1, 2}}};
  auto mesh = std::make_shared<MeshCollider>(vertices, triangles, 0.4);

  plane->setName("Suelo");
  sphere->setName("Esfera");
  capsule->setName("Capsula");
  mesh->setName("Malla");

  world.addCollider(plane);
  world.addCollider(sphere);
  world.addCollider(capsule);
  world.addCollider(mesh);

  world.moveCollider(2, Eigen::Vector3d(4.0, 4.0, 4.0), Eigen::Quaterniond::Identity());
  world.moveCollider(3, Eigen::Vector3d(1.0, 2.0, 3.0), Eigen::Quaterniond(0.707, 0.0, 0.707, 0.0).normalized());

  StateSerializer::save(m_tempFile.string(), solver, world);

  Solver loadedSolver;
  World loadedWorld;

  auto loadedPlane = std::make_shared<PlaneCollider>(Eigen::Vector3d(0.0, 0.0, 0.0),
                                                      Eigen::Vector3d(0.0, 1.0, 0.0), 0.5);
  auto loadedSphere = std::make_shared<SphereCollider>(Eigen::Vector3d(1.0, 1.0, 1.0),
                                                      0.5, 0.3);
  auto loadedCapsule = std::make_shared<CapsuleCollider>(0.25, Eigen::Vector3d(2.0, 2.0, 2.0), Eigen::Vector3d(3.0, 3.0, 3.0), 0.2);
  auto loadedMesh = std::make_shared<MeshCollider>(vertices, triangles, 0.4);

  loadedWorld.addCollider(loadedPlane);
  loadedWorld.addCollider(loadedSphere);
  loadedWorld.addCollider(loadedCapsule);
  loadedWorld.addCollider(loadedMesh);

  StateSerializer::load(m_tempFile.string(), loadedSolver, loadedWorld);

  const auto& colliders = world.getColliders();
  const auto& loadedColliders = loadedWorld.getColliders();

  EXPECT_EQ(colliders.size(), loadedColliders.size());
  for (size_t idx = 0; idx < colliders.size(); idx++) {
    EXPECT_EQ(typeid(*colliders[idx]), typeid(*loadedColliders[idx]));
    EXPECT_EQ(colliders[idx]->getName(), loadedColliders[idx]->getName());
    EXPECT_NEAR(colliders[idx]->getFriction(), loadedColliders[idx]->getFriction(), 1e-9);

    if (auto* p = dynamic_cast<PlaneCollider*>(colliders[idx].get())) {
      auto* loadedP = dynamic_cast<PlaneCollider*>(loadedColliders[idx].get());
      EXPECT_TRUE(p->getOrigin().isApprox(loadedP->getOrigin()));
      EXPECT_TRUE(p->getNormal().isApprox(loadedP->getNormal()));
    } else if (auto* s = dynamic_cast<SphereCollider*>(colliders[idx].get())) {
      auto* loadedS = dynamic_cast<SphereCollider*>(loadedColliders[idx].get());
      EXPECT_TRUE(s->getCenter().isApprox(loadedS->getCenter()));
      EXPECT_NEAR(s->getRadius(), loadedS->getRadius(), 1e-9);
    } else if (auto* c = dynamic_cast<CapsuleCollider*>(colliders[idx].get())) {
      auto* loadedC = dynamic_cast<CapsuleCollider*>(loadedColliders[idx].get());
      EXPECT_TRUE(c->getStart().isApprox(loadedC->getStart()));
      EXPECT_TRUE(c->getEnd().isApprox(loadedC->getEnd()));
      EXPECT_NEAR(c->getRadius(), loadedC->getRadius(), 1e-9);
    } else if (auto* m = dynamic_cast<MeshCollider*>(colliders[idx].get())) {
      auto* loadedM = dynamic_cast<MeshCollider*>(loadedColliders[idx].get());
      EXPECT_TRUE(m->getPosition().isApprox(loadedM->getPosition()));
      EXPECT_TRUE(m->getRotation().isApprox(loadedM->getRotation()));
      for (size_t v = 0; v < m->getWorldVertices().size(); ++v) {
        EXPECT_TRUE(m->getWorldVertices()[v].isApprox(loadedM->getWorldVertices()[v]));
      }
    }
  }
}
