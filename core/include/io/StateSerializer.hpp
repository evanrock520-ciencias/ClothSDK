#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "engine/World.hpp"
#include "physics/Particle.hpp"
#include "physics/Solver.hpp"

namespace Tissu {

struct StateInfo {
  uint8_t version;
  uint32_t frame;
  double timestamp;
  int particleCount;
};

class StateSerializer {
 public:
  static bool save(const std::string& path, Solver& solver, World& world);

  static bool load(const std::string& path, Solver& solver, World& world);

  static bool validate(const std::string& path);

  static StateInfo getStateInfo(const std::string& path);

 private:
  static constexpr uint8_t MAGIC[6] = {'T', 'I', 'S', 'S', 'U', '\0'};
  static constexpr uint8_t VERSION = 1;
  static constexpr uint8_t FLAGS = 0;
  static constexpr size_t HEADER_SIZE = 32;
  static constexpr uint32_t CRC32_POLY = 0xEDB88320;

  static void initCrcTable();
  static uint32_t computeCRC32(const uint8_t* data, size_t length);
};

}  // namespace Tissu