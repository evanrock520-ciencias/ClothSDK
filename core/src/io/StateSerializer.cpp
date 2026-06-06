#include "io/StateSerializer.hpp"
#include "engine/World.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/Collider.hpp"
#include "physics/Particle.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/Solver.hpp"
#include "physics/SphereCollider.hpp"
#include "utils/Logger.hpp"
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>

static uint32_t buildCrcTable[256] = {};
static bool crcTableReady = false;

namespace Tissu {

bool StateSerializer::save(const std::string& path, Solver& solver, World& world) {
    std::vector<uint8_t> buf;
    buf.reserve(1024 * 1024); 

    auto write = [&](const void* data, size_t size) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(data);
        buf.insert(buf.end(), ptr, ptr + size);
    };

    // Header
    write(MAGIC, 6);
    write(&VERSION, 1);
    write(&FLAGS, 1);
    uint32_t frame = solver.getCurrentFrame();
    double   time  = solver.getCurrentTime();
    uint32_t count = solver.getParticleCount();
    write(&frame, sizeof(uint32_t));
    write(&time,  sizeof(double));
    write(&count, sizeof(uint32_t));

    size_t crcOffset = buf.size();
    uint32_t crcPlaceholder = 0x00000000;
    write(&crcPlaceholder, sizeof(uint32_t));

    uint32_t padding = 0x00000000;
    write(&padding, sizeof(uint32_t));

    // World
    Eigen::Vector3d gravity = world.getGravity();
    Eigen::Vector3d wind = world.getWind();
    double density = world.getAirDensity();
    double thickness = world.getThickness();
    write(gravity.data(), 3 * sizeof(double));
    write(wind.data(), 3 * sizeof(double));
    write(&density, sizeof(double));
    write(&thickness, sizeof(double));

    // Colliders
    const auto& colliders = world.getColliders();
    uint32_t colliderCount = static_cast<uint32_t>(colliders.size());
    write(&colliderCount, sizeof(uint32_t));

    for (const auto& collider : colliders) {
        double friction = collider->getFriction();
        if (const auto* s = dynamic_cast<const SphereCollider*>(collider.get())) {
            uint8_t type = 0;
            Eigen::Vector3d center = s->getCenter();
            double radius = s->getRadius();
            write(&type, sizeof(uint8_t));
            write(&friction, sizeof(double));
            write(center.data(), 3 * sizeof(double));
            write(&radius, sizeof(double));
        } else if (const auto* p = dynamic_cast<const PlaneCollider*>(collider.get())) {
            uint8_t type = 1;
            Eigen::Vector3d origin = p->getOrigin();
            Eigen::Vector3d normal = p->getNormal();
            write(&type, sizeof(uint8_t));
            write(&friction, sizeof(double));
            write(origin.data(), 3 * sizeof(double));
            write(normal.data(), 3 * sizeof(double));
        } else if (const auto* c = dynamic_cast<const CapsuleCollider*>(collider.get())) {
            uint8_t type = 2;
            Eigen::Vector3d start = c->getStart();
            Eigen::Vector3d end = c->getEnd();
            double radius = c->getRadius();
            write(&type, sizeof(uint8_t));
            write(&friction, sizeof(double));
            write(start.data(), 3 * sizeof(double));
            write(end.data(), 3 * sizeof(double));
            write(&radius, sizeof(double));
        }
    }

    // Particles
    const auto& particles = solver.getParticles();
    for (const auto& p : particles) {
        Eigen::Vector3d pos = p.getPosition();
        Eigen::Vector3d oldPos = p.getOldPosition();
        double invMass = p.getInverseMass();
        write(pos.data(), 3 * sizeof(double));
        write(oldPos.data(), 3 * sizeof(double));
        write(&invMass, sizeof(double));
    }

    // Lambdas
    const auto& constraints = solver.getConstraints();
    uint32_t constraintCount = static_cast<uint32_t>(constraints.size());
    write(&constraintCount, sizeof(uint32_t));
    for (const auto& c : constraints) {
        double lambda = c->getLambda();
        write(&lambda, sizeof(double));
    }

    uint32_t crc = computeCRC32(buf.data(), buf.size());
    memcpy(buf.data() + crcOffset, &crc, sizeof(uint32_t));

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(buf.data()), buf.size());

    return file.good();
}

bool StateSerializer::load(const std::string &path, Solver& solver, World& world) {
    Logger::info("Loading state...");
    return true;
}

void StateSerializer::initCrcTable() {
    if (crcTableReady) return;
    for (uint32_t idx = 0; idx < 256; idx++) {
        uint32_t crc = idx;
        for (int jdx = 0; jdx < 8; jdx++) {
            if (crc & 1)
                crc = (crc >> 1) ^ CRC32_POLY;
            else
                crc >>= 1;
        }
        buildCrcTable[idx] = crc;
    }
    crcTableReady = true;
}

uint32_t StateSerializer::computeCRC32(const uint8_t* data, size_t length) {
    initCrcTable();
    uint32_t crc = 0xFFFFFFFF;
    for (size_t idx = 0; idx < length; idx++)
        crc = (crc >> 8) ^ buildCrcTable[(crc ^ data[idx]) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

}