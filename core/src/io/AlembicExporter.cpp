// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "io/AlembicExporter.hpp"
#include "utils/Logger.hpp"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreOgawa/All.h>
#include <Alembic/Abc/ErrorHandler.h>

namespace ClothSDK {

using namespace Alembic::AbcGeom;

struct AlembicExporter::Impl {
    std::unique_ptr<OArchive> archive;
    std::unique_ptr<OXform> xform;
    std::unique_ptr<OPolyMesh> mesh;
    
    OPolyMeshSchema schema;
};

AlembicExporter::AlembicExporter() : m_impl(std::make_unique<Impl>()) {}
AlembicExporter::~AlembicExporter() = default;

bool AlembicExporter::open(const std::string& path, 
                            const std::vector<Eigen::Vector3d>& positions, 
                            const std::vector<int>& indices) {
    try {
        m_impl->archive = std::make_unique<OArchive>(Alembic::AbcCoreOgawa::WriteArchive(), path);
        
        double dt = 1.0 / 60.0;
        Abc::TimeSampling ts(dt, 0.0);
        uint32_t tsIndex = m_impl->archive->addTimeSampling(ts);

        m_impl->xform = std::make_unique<OXform>(*m_impl->archive, "cloth_xform", tsIndex);
        m_impl->mesh = std::make_unique<OPolyMesh>(*m_impl->xform, "cloth_mesh", tsIndex);
        m_impl->schema = m_impl->mesh->getSchema();

        std::vector<Imath::V3f> initialPos;
        initialPos.reserve(positions.size());
        for (const auto& p : positions) {
            initialPos.emplace_back(static_cast<float>(p.x()), 
                                    static_cast<float>(p.y()), 
                                    static_cast<float>(p.z()));
        }

        std::vector<int32_t> faceCounts(indices.size() / 3, 3);

        OPolyMeshSchema::Sample initialSample;
        initialSample.setPositions(Abc::V3fArraySample(initialPos.data(), initialPos.size()));
        initialSample.setFaceIndices(Abc::Int32ArraySample(indices.data(), indices.size()));
        initialSample.setFaceCounts(Abc::Int32ArraySample(faceCounts.data(), faceCounts.size()));
        
        m_impl->schema.set(initialSample);

        return true;
    } catch (const std::exception& e) {
        Logger::error("Alembic Exception: " + std::string(e.what()));
        return false;
    }
}

void AlembicExporter::writeFrame(const std::vector<Eigen::Vector3d>& positions, double time) {
    if (!m_impl->mesh) return;

    std::vector<Imath::V3f> alembicPos;
    alembicPos.reserve(positions.size());

    for (const auto& p : positions) {
        alembicPos.emplace_back(static_cast<float>(p.x()), 
                                static_cast<float>(p.y()), 
                                static_cast<float>(p.z()));
    }

    OPolyMeshSchema::Sample frameSample;
    frameSample.setPositions(Abc::V3fArraySample(alembicPos.data(), alembicPos.size()));

    m_impl->schema.set(frameSample);
}

void AlembicExporter::close() {
    m_impl->mesh.reset();
    m_impl->xform.reset();
    m_impl->archive.reset();
}

} 