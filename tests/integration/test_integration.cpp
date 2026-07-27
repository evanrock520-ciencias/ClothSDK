#include <gtest/gtest.h>

#include <filesystem>
#include <memory>

#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"
#include "io/SceneExporter.hpp"
#include "io/SceneLoader.hpp"
#include "io/StateSerializer.hpp"
#include "math/Types.hpp"
#include "physics/GravityForce.hpp"
#include "physics/Solver.hpp"

using namespace Tissu;
namespace fs = std::filesystem;

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

    for (int idx = 0; idx < 120; idx++)
        solver.update(world, 1.0 / 60.0);

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

    for (int idx = 0; idx < 300; idx++)
        solver.update(world, 1.0 / 60.0);

    std::vector<Particle> after = solver.getParticles();

    for (size_t idx = 0; idx < initial.size(); idx++)
        EXPECT_LT(after[idx].getPosition().y(), initial[idx].getPosition().y());
}

class IntegrationIOTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_sceneFile =
            fs::temp_directory_path() / "tissu_integration_scene.json";
        m_stateFile =
            fs::temp_directory_path() / "tissu_integration_state.tissu";
    }

    void TearDown() override {
        if (fs::exists(m_sceneFile))
            fs::remove(m_sceneFile);
        if (fs::exists(m_stateFile))
            fs::remove(m_stateFile);
    }

    fs::path m_sceneFile;
    fs::path m_stateFile;
};

TEST_F(IntegrationIOTest, SaveSceneAndSaveState) {
    Solver solver;
    World world;

    auto material = std::make_shared<ClothMaterial>();
    const auto cloth = std::make_shared<Cloth>("test", material);

    ClothMesh mesh;
    mesh.initGrid(50, 50, 0.05, *cloth, solver);

    const auto gravity =
        std::make_shared<GravityForce>(Eigen::Vector3d(0.0, -9.81, 0.0));
    world.addForce(gravity);
    world.addCloth(cloth);

    for (int idx = 0; idx < 60; idx++)
        solver.update(world, 1.0 / 60.0);

    SceneExporter::saveScene(m_sceneFile.string(), "integration_test", solver,
                             world);
    StateSerializer::save(m_stateFile.string(), solver, world);

    EXPECT_TRUE(fs::exists(m_sceneFile));
    EXPECT_TRUE(fs::exists(m_stateFile));

    Solver loadedSolver;
    World loadedWorld;
    SceneLoader::loadScene(m_sceneFile.string(), loadedSolver, loadedWorld);
    StateSerializer::load(m_stateFile.string(), loadedSolver, loadedWorld);

    const auto& original = solver.getParticles();
    const auto& loaded = loadedSolver.getParticles();

    ASSERT_EQ(original.size(), loaded.size());
    for (size_t idx = 0; idx < original.size(); idx++) {
        EXPECT_TRUE(
            original[idx].getPosition().isApprox(loaded[idx].getPosition()));
    }
}