// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/AlembicExporter.hpp"

#include <Alembic/Abc/ErrorHandler.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/AbcGeom/All.h>

#include "utils/Logger.hpp"

namespace Tissu {

using namespace Alembic::AbcGeom;

struct ExportedMesh {
  std::unique_ptr<OXform> xform;
  std::unique_ptr<OPolyMesh> mesh;

  OPolyMeshSchema schema;
  std::vector<int> particle_indices;
};

struct AlembicExporter::Impl {
  std::unique_ptr<OArchive> archive;
  std::vector<ExportedMesh> meshes;
};

AlembicExporter::AlembicExporter() : m_impl(std::make_unique<Impl>()) {}
AlembicExporter::~AlembicExporter() = default;

bool AlembicExporter::open(const std::string& path,
                           const std::vector<std::string>& names,
                           const std::vector<Eigen::Vector3d>& global_positions,
                           const std::vector<std::vector<int>>& global_indices,
                           const std::vector<std::vector<int>>& particle_indices) {
  try {
    m_impl->archive =
        std::make_unique<OArchive>(Alembic::AbcCoreOgawa::WriteArchive(), path);

    double dt = 1.0 / 60.0;
    Abc::TimeSampling ts(dt, 0.0);
    uint32_t tsIndex = m_impl->archive->addTimeSampling(ts);

    m_impl->meshes.clear();
    m_impl->meshes.reserve(names.size());

    for (size_t i = 0; i < names.size(); ++i) {
      const auto& name = names[i];
      const auto& global_idx = global_indices[i];
      const auto& part_indices = particle_indices[i];

      ExportedMesh em;
      em.xform = std::make_unique<OXform>(*m_impl->archive, name + "_xform", tsIndex);
      em.mesh = std::make_unique<OPolyMesh>(*em.xform, name + "_mesh", tsIndex);
      em.schema = em.mesh->getSchema();
      em.particle_indices = part_indices;

      // Extract local positions from global positions
      std::vector<Imath::V3f> initialPos;
      initialPos.reserve(part_indices.size());
      for (int gid : part_indices) {
        const auto& p = global_positions[gid];
        initialPos.emplace_back(static_cast<float>(p.x()),
                                static_cast<float>(p.y()),
                                static_cast<float>(p.z()));
      }

      // Map global indices to local 0-based indices for this cloth mesh
      int max_id = 0;
      for (int id : part_indices) {
        if (id > max_id) max_id = id;
      }
      std::vector<int> global_to_local(max_id + 1, -1);
      for (size_t local_idx = 0; local_idx < part_indices.size(); ++local_idx) {
        global_to_local[part_indices[local_idx]] = static_cast<int>(local_idx);
      }

      std::vector<int32_t> local_indices;
      local_indices.reserve(global_idx.size());
      for (int gid : global_idx) {
        int lid = global_to_local[gid];
        if (lid == -1) {
          lid = 0;
        }
        local_indices.push_back(lid);
      }

      std::vector<int32_t> faceCounts(local_indices.size() / 3, 3);

      OPolyMeshSchema::Sample initialSample;
      initialSample.setPositions(
          Abc::V3fArraySample(initialPos.data(), initialPos.size()));
      initialSample.setFaceIndices(
          Abc::Int32ArraySample(local_indices.data(), local_indices.size()));
      initialSample.setFaceCounts(
          Abc::Int32ArraySample(faceCounts.data(), faceCounts.size()));

      em.schema.set(initialSample);

      m_impl->meshes.push_back(std::move(em));
    }

    return true;
  } catch (const std::exception& e) {
    Logger::error("Alembic Exception: " + std::string(e.what()));
    return false;
  }
}

void AlembicExporter::writeFrame(const std::vector<Eigen::Vector3d>& global_positions,
                                 double time) {
  for (size_t i = 0; i < m_impl->meshes.size(); ++i) {
    auto& em = m_impl->meshes[i];

    std::vector<Imath::V3f> alembicPos;
    alembicPos.reserve(em.particle_indices.size());

    for (int gid : em.particle_indices) {
      const auto& p = global_positions[gid];
      alembicPos.emplace_back(static_cast<float>(p.x()),
                              static_cast<float>(p.y()),
                              static_cast<float>(p.z()));
    }

    OPolyMeshSchema::Sample frameSample;
    frameSample.setPositions(
        Abc::V3fArraySample(alembicPos.data(), alembicPos.size()));

    em.schema.set(frameSample);
  }
}

void AlembicExporter::close() {
  m_impl->meshes.clear();
  m_impl->archive.reset();
}

}  // namespace Tissu