#include "engine/Cloth.hpp"
#include "engine/ClothMesh.hpp"
#include "engine/World.hpp"
#include "io/AlembicExporter.hpp"
#include "physics/GravityForce.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/Solver.hpp"

#include "Eigen/Dense"

using namespace Tissu;

int main() {
    World world;
    Solver solver;
    auto material = std::make_shared<ClothMaterial>(0.1, 1e-9, 1e-9, 0.05);
    const auto curtain = std::make_shared<Cloth>("curtain", material);

    ClothMesh mesh;
    mesh.initGrid(80, 80, 0.05, *curtain, solver);

    const auto gravity =
        std::make_shared<GravityForce>(Eigen::Vector3d(0.0, -9.81, 0.0));
    world.addForce(gravity);
    world.addCloth(curtain);

    const auto plane = std::make_shared<PlaneCollider>(
        Eigen::Vector3d(0.0, 0.0, 0.0), Eigen::Vector3d(0.0, 1.0, 0.0), 0.5);
    world.addCollider(plane);

    std::vector<std::string> names;
    std::vector<std::vector<int>> global_indices;
    std::vector<std::vector<int>> particle_indices;

    for (const auto& cloth : world.getCloths()) {
        names.push_back(cloth->getName());
        std::vector<int> flat_triangles;
        for (const auto& tri : cloth->getTriangles()) {
            flat_triangles.push_back(tri.a);
            flat_triangles.push_back(tri.b);
            flat_triangles.push_back(tri.c);
        }
        global_indices.push_back(flat_triangles);
        particle_indices.push_back(cloth->getParticleIndices());
    }

    std::vector<Eigen::Vector3d> global_positions;
    for (const auto& p : solver.getParticles()) {
        global_positions.push_back(p.getPosition());
    }

    AlembicExporter exporter;
    const bool success =
        exporter.open("data/animations/curtain.abc", names, global_positions,
                      global_indices, particle_indices);

    if (!success)
        return -1;

    constexpr int total_frames = 100;
    constexpr double dt = 1.0 / 60.0;

    for (int frame = 0; frame < total_frames; ++frame) {
        solver.update(world, dt);
        std::vector<Eigen::Vector3d> current_positions;
        for (const auto& p : solver.getParticles()) {
            current_positions.push_back(p.getPosition());
        }
        exporter.writeFrame(current_positions, frame * dt);
    }

    exporter.close();
    return 0;
}