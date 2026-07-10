#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>

#include "engine/World.hpp"
#include "io/ConfigLoader.hpp"
#include "math/Types.hpp"
#include "physics/Solver.hpp"

using namespace Tissu;
namespace fs = std::filesystem;

class ConfigLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        m_tempFile = fs::temp_directory_path() / "tissu_test_config.json";
    }

    void TearDown() override {
        if (fs::exists(m_tempFile))
            fs::remove(m_tempFile);
    }

    fs::path m_tempFile;
};

TEST_F(ConfigLoaderTest, LoadMaterialThrowsIfFileNotFound) {
    ClothMaterial mat;
    EXPECT_THROW(ConfigLoader::loadMaterial("nonexistent.json", mat),
                 std::runtime_error);
}

TEST_F(ConfigLoaderTest, LoadMaterialThrowsIfTypeIsIncorrect) {
    std::ofstream file(m_tempFile);
    file << R"({ "version": "2.0", "type": "physics", "name": "test" })";
    file.close();

    ClothMaterial mat;
    EXPECT_THROW(ConfigLoader::loadMaterial(m_tempFile.string(), mat),
                 std::invalid_argument);
}

TEST_F(ConfigLoaderTest, SaveAndLoadMaterialRoundTrip) {
    ClothMaterial original;
    original.setDensity(0.12);
    original.setBendingCompliance(0.01);
    original.setShearCompliance(0.2);
    original.setStructuralCompliance(0.6);

    ConfigLoader::saveMaterial(m_tempFile.string(), original, "fur");

    ClothMaterial loaded;
    ConfigLoader::loadMaterial(m_tempFile.string(), loaded);

    EXPECT_NEAR(loaded.getDensity(), original.getDensity(), 1e-9);
    EXPECT_NEAR(loaded.getBendingCompliance(), original.getBendingCompliance(),
                1e-9);
    EXPECT_NEAR(loaded.getShearCompliance(), original.getShearCompliance(),
                1e-9);
    EXPECT_NEAR(loaded.getStructuralCompliance(),
                original.getStructuralCompliance(), 1e-9);
}

TEST_F(ConfigLoaderTest, LoadPhysicsThrowsIfFileNotFound) {
    World world;
    Solver solver;
    EXPECT_THROW(ConfigLoader::loadPhysics("nonexistent.json", solver, world),
                 std::runtime_error);
}

TEST_F(ConfigLoaderTest, LoadPhysicsThrowsIfTypeIsIncorrect) {
    std::ofstream file(m_tempFile);
    file << R"({ "version": "2.0", "type": "material", "name": "test" })";
    file.close();

    Solver solver;
    World world;

    EXPECT_THROW(ConfigLoader::loadPhysics(m_tempFile.string(), solver, world),
                 std::invalid_argument);
}

TEST_F(ConfigLoaderTest, SaveAndLoadPhysicsRoundTrip) {
    Solver solver;
    World world;

    solver.setSubsteps(20);
    solver.setIterations(4);
    world.setGravity(Eigen::Vector3d(0.0, -9.81, 0.0));
    world.setThickness(0.03);
    world.setWind(Eigen::Vector3d(5.0, 0.0, 0.0));
    world.setAirDensity(0.2);

    ConfigLoader::savePhysics(m_tempFile.string(), solver, world, "test");

    Solver loadedSolver;
    World loadedWorld;
    ConfigLoader::loadPhysics(m_tempFile.string(), loadedSolver, loadedWorld);

    EXPECT_EQ(loadedSolver.getSubsteps(), solver.getSubsteps());
    EXPECT_EQ(loadedSolver.getIterations(), solver.getIterations());
    EXPECT_NEAR(loadedWorld.getThickness(), world.getThickness(), 1e-9);
    EXPECT_NEAR(loadedWorld.getGravity().y(), world.getGravity().y(), 1e-9);
    EXPECT_NEAR(loadedWorld.getWind().x(), world.getWind().x(), 1e-9);
    EXPECT_NEAR(loadedWorld.getAirDensity(), world.getAirDensity(), 1e-9);
}