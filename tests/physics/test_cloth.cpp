#include "physics/Solver.hpp"
#include "utils/Logger.hpp"

#include <Eigen/Dense>
#include <engine/Cloth.hpp>
#include <engine/ClothMesh.hpp>
#include <gtest/gtest.h>

using namespace Tissu;

TEST(Cloth, SettersAndGetters) {
    auto material = std::make_shared<ClothMaterial>();
    const auto cloth = std::make_shared<Cloth>("test", material);

    // Values
    const auto name = "Scarf";
    constexpr auto cols = 50;
    constexpr auto rows = 50;
    constexpr auto restVolume = 100.0;
    const Eigen::Quaterniond rotation(0.717, 0.0, 0.0, 0.717);
    const Eigen::Vector3d translation(8, 10, 0);
    constexpr auto spacing = 0.1;

    cloth->setName(name);
    cloth->setGridDimensions(rows, cols);
    cloth->setRestVolume(restVolume);
    cloth->setRotation(rotation);
    cloth->setTranslation(translation);
    cloth->setSpacing(spacing);

    EXPECT_EQ(name, cloth->getName());
    EXPECT_EQ(cols, cloth->getCols());
    EXPECT_EQ(rows, cloth->getRows());
    EXPECT_NEAR(restVolume, cloth->getRestVolume(), 1e-4);
    EXPECT_NEAR(spacing, cloth->getSpacing(), 1e-4);
    EXPECT_EQ(rotation, cloth->getRotation());
    EXPECT_EQ(translation, cloth->getTranslation());
}

TEST(Cloth, ClearFabric) {
    Solver solver;

    auto material = std::make_shared<ClothMaterial>();
    const auto cloth = std::make_shared<Cloth>("test", material);
    ClothMesh mesh;
    mesh.initGrid(50, 50, 0.05, *cloth, solver);

    EXPECT_GT(cloth->getTriangles().size(), 0);
    EXPECT_GT(cloth->getParticleIndices().size(), 0);
    EXPECT_GT(cloth->getAeroFaces().size(), 0);
    EXPECT_GT(cloth->getVisualEdges().size(), 0);

    cloth->clear();
    EXPECT_EQ(cloth->getTriangles().size(), 0);
    EXPECT_EQ(cloth->getParticleIndices().size(), 0);
    EXPECT_EQ(cloth->getAeroFaces().size(), 0);
    EXPECT_EQ(cloth->getVisualEdges().size(), 0);
}