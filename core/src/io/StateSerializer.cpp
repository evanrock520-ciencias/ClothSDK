#include "io/StateSerializer.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "engine/World.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/Collider.hpp"
#include "physics/Particle.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/Solver.hpp"
#include "physics/SphereCollider.hpp"
#include "utils/Logger.hpp"

static uint32_t buildCrcTable[256] = {};
static bool crcTableReady = false;

namespace Tissu {

bool StateSerializer::save(const std::string& path, Solver& solver,
                           World& world) {
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
  double time = solver.getCurrentTime();
  uint32_t count = solver.getParticleCount();
  write(&frame, sizeof(uint32_t));
  write(&time, sizeof(double));
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
    } else if (const auto* p =
                   dynamic_cast<const PlaneCollider*>(collider.get())) {
      uint8_t type = 1;
      Eigen::Vector3d origin = p->getOrigin();
      Eigen::Vector3d normal = p->getNormal();
      write(&type, sizeof(uint8_t));
      write(&friction, sizeof(double));
      write(origin.data(), 3 * sizeof(double));
      write(normal.data(), 3 * sizeof(double));
    } else if (const auto* c =
                   dynamic_cast<const CapsuleCollider*>(collider.get())) {
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
  if (!file.is_open()) throw std::runtime_error("Invalid directory.");
  file.write(reinterpret_cast<const char*>(buf.data()), buf.size());

  return file.good();
}

bool StateSerializer::load(const std::string& path, Solver& solver,
                           World& world) {
  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) throw std::runtime_error("File not found.");

  std::vector<uint8_t> buf(std::istreambuf_iterator<char>(file), {});
  if (buf.size() < 32) return false;

  size_t cursor = 0;
  auto read = [&](void* dst, size_t size) -> bool {
    if (cursor + size > buf.size()) return false;
    memcpy(dst, buf.data() + cursor, size);
    cursor += size;
    return true;
  };

  if (memcmp(buf.data(), MAGIC, 6) != 0) {
    Logger::error("Invalid magic number.");
    return false;
  }

  constexpr size_t CRC_OFFSET = 24;
  uint32_t storedCrc = 0;
  memcpy(&storedCrc, buf.data() + CRC_OFFSET, sizeof(uint32_t));

  memset(buf.data() + CRC_OFFSET, 0, sizeof(uint32_t));
  uint32_t computedCrc = computeCRC32(buf.data(), buf.size());
  memcpy(buf.data() + CRC_OFFSET, &storedCrc, sizeof(uint32_t));

  if (storedCrc != computedCrc) {
    Logger::error("CRC mismatch.");
    return false;
  }

  // Header
  cursor = 6;
  uint8_t version, flags;
  uint32_t frame, count;
  double time;

  if (!read(&version, 1)) return false;
  if (!read(&flags, 1)) return false;
  if (!read(&frame, sizeof(uint32_t))) return false;
  if (!read(&time, sizeof(double))) return false;
  if (!read(&count, sizeof(uint32_t))) return false;

  cursor += sizeof(uint32_t);
  cursor += sizeof(uint32_t);

  if (version != VERSION) {
    Logger::error("Version mismatch");
    return false;
  }

  // World
  Eigen::Vector3d gravity, wind;
  double density, thickness;

  if (!read(&gravity, 3 * sizeof(double))) return false;
  if (!read(&wind, 3 * sizeof(double))) return false;
  if (!read(&density, sizeof(double))) return false;
  if (!read(&thickness, sizeof(double))) return false;

  world.setGravity(gravity);
  world.setWind(wind);
  world.setAirDensity(density);
  world.setThickness(thickness);

  // Colliders
  uint32_t collidersCount;
  if (!read(&collidersCount, sizeof(uint32_t))) return false;

  for (size_t idx = 0; idx < collidersCount; idx++) {
    uint8_t type;
    double friction;

    if (!read(&type, sizeof(uint8_t))) return false;
    if (!read(&friction, sizeof(double))) return false;

    switch (type) {
      case 0: {
        Eigen::Vector3d center;
        double radius;

        if (!read(&center, 3 * sizeof(double))) return false;
        if (!read(&radius, sizeof(double))) return false;

        world.addSphereCollider(center, radius, friction);
        break;
      }
      case 1: {
        Eigen::Vector3d origin, normal;

        if (!read(&origin, 3 * sizeof(double))) return false;
        if (!read(&normal, 3 * sizeof(double))) return false;

        world.addPlaneCollider(origin, normal, friction);
        break;
      }
      case 2: {
        Eigen::Vector3d start, end;
        double radius;

        if (!read(&start, 3 * sizeof(double))) return false;
        if (!read(&end, 3 * sizeof(double))) return false;
        if (!read(&radius, sizeof(double))) return false;

        world.addCapsuleCollider(start, end, radius, friction);
        break;
      }
    }
  }

  // Particles
  auto& particles = solver.getParticles();
  if (particles.size() != count) {
    Logger::error("Particles size mistmatch: '" + std::to_string(count) +
                  "' rather than '" + std::to_string(particles.size()) + "'");
    return false;
  }

  for (size_t idx = 0; idx < count; ++idx) {
    Eigen::Vector3d pos, oldPos;
    double invMass;

    if (!read(pos.data(), 3 * sizeof(double))) return false;
    if (!read(oldPos.data(), 3 * sizeof(double))) return false;
    if (!read(&invMass, sizeof(double))) return false;

    particles[idx].setPosition(pos);
    particles[idx].setOldPosition(oldPos);
    particles[idx].setInverseMass(invMass);
  }

  // Constraints
  uint32_t constraintsCount;

  if (!read(&constraintsCount, sizeof(uint32_t))) return false;

  auto& constraints = solver.getConstraints();
  if (constraints.size() != constraintsCount) {
    Logger::error("Constraints size mistmatch: '" +
                  std::to_string(constraintsCount) + "' rather than '" +
                  std::to_string(constraints.size()) + "'");
    return false;
  }

  for (size_t idx = 0; idx < constraintsCount; idx++) {
    double lambda;
    if (!read(&lambda, sizeof(double))) return false;

    constraints[idx]->setLambda(lambda);
  }
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

}  // namespace Tissu