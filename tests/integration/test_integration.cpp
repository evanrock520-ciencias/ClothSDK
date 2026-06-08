#include <gtest/gtest.h>

#include <memory>

#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"
#include "math/Types.hpp"
#include "physics/GravityForce.hpp"
#include "physics/Solver.hpp"

using namespace Tissu;

TEST(Integration, SimulationRuns120FramesWithoutNaN) {
  World world;
  Solver solver;

  auto material = std::make_shared<ClothMaterial>();
  auto cloth = std::make_shared<Cloth>("test", material);

  ClothMesh mesh;
  mesh.initGrid(50, 50, 0.1, *cloth, solver);

  auto gravity =
      std::make_shared<GravityForce>(Eigen::Vector3d(0.0, -9.81, 0.0));
  world.addForce(gravity);
  world.addCloth(cloth);

  for (int idx = 0; idx < 120; idx++) solver.update(world, 1.0 / 60.0);

  for (const auto& p : solver.getParticles()) {
    EXPECT_FALSE(std::isnan(p.getPosition().x()));
    EXPECT_FALSE(std::isnan(p.getPosition().y()));
    EXPECT_FALSE(std::isnan(p.getPosition().z()));
  }
}

TEST(Integration, ClothFallsUnderGravity) {
  World world;
  Solver solver;

  auto material = std::make_shared<ClothMaterial>();
  auto cloth = std::make_shared<Cloth>("test", material);

  ClothMesh mesh;
  mesh.initGrid(50, 50, 0.05, *cloth, solver);

  std::vector<Particle> initial = solver.getParticles();

  auto gravity =
      std::make_shared<GravityForce>(Eigen::Vector3d(0.0, -9.81, 0.0));

  world.addForce(gravity);
  world.addCloth(cloth);

  for (int idx = 0; idx < 300; idx++) solver.update(world, 1.0 / 60.0);

  std::vector<Particle> after = solver.getParticles();

  for (size_t idx = 0; idx < initial.size(); idx++)
    EXPECT_LT(after[idx].getPosition().y(), initial[idx].getPosition().y());
}