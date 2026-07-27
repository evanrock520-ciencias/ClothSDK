#include "Eigen/Dense"
#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"
#include "physics/GravityForce.hpp"
#include "physics/Particle.hpp"
#include "physics/Solver.hpp"
#include <benchmark/benchmark.h>
#include <cmath>
#include <vector>

using namespace Tissu;

static void BM_Solver(benchmark::State& state) {
    const int gridSize = state.range(0);

    World world;
    Solver solver;

    auto material = std::make_shared<ClothMaterial>();
    const auto cloth = std::make_shared<Cloth>("test", material);

    ClothMesh mesh;
    mesh.initGrid(gridSize, gridSize, 0.05, *cloth, solver);

    std::vector<Particle> initial = solver.getParticles();

    auto gravity =
        std::make_shared<GravityForce>(Eigen::Vector3d(0.0, -9.81, 0.0));

    world.addForce(gravity);
    world.addCloth(cloth);

    for (auto _ : state) {
        for (int idx = 0; idx < 300; idx++) {
            solver.update(world, 1.0 / 60.0);
        }
    }

    state.SetItemsProcessed(state.iterations() * 300);
    state.counters["ParticlesPerFrame"] = static_cast<double>(initial.size());
    state.counters["GridSize"] = gridSize;
}

BENCHMARK(BM_Solver)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10)
    ->Arg(25)
    ->Arg(50)
    ->Arg(100)
    ->Arg(200);
BENCHMARK_MAIN();