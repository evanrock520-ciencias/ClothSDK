#include "Eigen/Dense"
#include "physics/AerodynamicForce.hpp"
#include "physics/Particle.hpp"
#include <benchmark/benchmark.h>
#include <cmath>
#include <vector>

static std::vector<Tissu::Particle> makeParticles(int count) {
    std::vector<Tissu::Particle> particles;
    particles.reserve(count);
    for (int idx = 0; idx < count; idx++) {
        double x = std::fmod(idx * 0.1, 10.0);
        double y = std::fmod(idx * 0.07, 10.0);
        double z = std::fmod(idx * 0.05, 10.0);
        particles.emplace_back(Eigen::Vector3d(x, y, z));
    }

    return particles;
}

static std::vector<Tissu::AeroFace> makeFaces(int count) {
    std::vector<Tissu::AeroFace> faces;
    faces.reserve(count);

    for (int idx = 0; idx < count; idx++) {
        int base = (idx * 3) % (count * 3);
        faces.push_back({base % count, (base + 1) % count, (base + 2) % count});
    }

    return faces;
}

static void BM_AerodynamicForce(benchmark::State& state) {
    int faceCount = state.range(0);
    int particleCount = faceCount * 3;

    auto particles = makeParticles(particleCount);
    auto faces = makeFaces(faceCount);

    Eigen::Vector3d wind(2.0, 0.0, 1.0);
    Tissu::AerodynamicForce aero(faces, wind, 1.2);

    for (auto _ : state) {
        state.PauseTiming();
        for (auto& particle : particles)
            particle.clearForces();
        state.ResumeTiming();

        aero.apply(particles, 1.0 / 60.0);
    }

    state.counters["faces"] = faceCount;
    state.counters["particles"] = particleCount;
}

static void BM_AerodynamicForce_Serial(benchmark::State& state) {
    int faceCount = state.range(0);
    int particleCount = faceCount * 3;

    auto particles = makeParticles(particleCount);
    auto faces = makeFaces(faceCount);

    Eigen::Vector3d wind(2.0, 0.0, 1.0);
    double dt = 1.0 / 60.0;

    for (auto _ : state) {
        state.PauseTiming();
        for (auto& p : particles)
            p.clearForces();
        state.ResumeTiming();

        for (int idx = 0; idx < (int)faces.size(); idx++) {
            const auto& face = faces[idx];
            Tissu::Particle& pA = particles[face.a];
            Tissu::Particle& pB = particles[face.b];
            Tissu::Particle& pC = particles[face.c];

            Eigen::Vector3d vFace =
                (pA.getVelocity(dt) + pB.getVelocity(dt) + pC.getVelocity(dt)) /
                3.0;
            Eigen::Vector3d vRel = vFace - wind;
            double vMag = vRel.norm();
            if (vMag < 1e-4)
                continue;

            Eigen::Vector3d edge1 = pB.getPosition() - pA.getPosition();
            Eigen::Vector3d edge2 = pC.getPosition() - pA.getPosition();
            Eigen::Vector3d n = edge1.cross(edge2);
            double area = 0.5 * n.norm();
            if (area < 1e-6)
                continue;

            Eigen::Vector3d normal = n.normalized();
            double pressure = vRel.dot(normal) / vMag;
            Eigen::Vector3d f =
                (-0.5 * 1.2 * vMag * vMag * area * pressure * normal) / 3.0;

            pA.addForce(f);
            pB.addForce(f);
            pC.addForce(f);
        }
    }

    state.counters["faces"] = faceCount;
}

BENCHMARK(BM_AerodynamicForce)->RangeMultiplier(4)->Range(512, 32768);
BENCHMARK(BM_AerodynamicForce_Serial)->RangeMultiplier(4)->Range(512, 32768);
BENCHMARK_MAIN();