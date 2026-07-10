// Copyright 2026 Evan M.
// SPDX-License-Identifier: Apache-2.0

#include "Renderer.hpp"

#include <cmath>
#include <glad/gl.h>

#include "Camera.hpp"
#include "Shader.hpp"
#include "engine/World.hpp"
#include "physics/CapsuleCollider.hpp"
#include "physics/Collider.hpp"
#include "physics/MeshCollider.hpp"
#include "physics/PlaneCollider.hpp"
#include "physics/Solver.hpp"
#include "physics/SphereCollider.hpp"

namespace Tissu {
namespace Viewer {

Renderer::Renderer() : m_shader("", ""), m_colliderShader("", "") {}
Renderer::~Renderer() {
    cleanup();
}

bool Renderer::init() {
    std::string vertPath = m_shaderPath + "cloth.vert";
    std::string fragPath = m_shaderPath + "cloth.frag";

    m_shader = Shader(vertPath, fragPath);
    if (!m_shader.init()) {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

    constexpr GLsizei stride = 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    std::string colliderVertPath = m_shaderPath + "collider.vert";
    std::string colliderFragPath = m_shaderPath + "collider.frag";
    m_colliderShader = Shader(colliderVertPath, colliderFragPath);
    if (!m_colliderShader.init()) {
        return false;
    }

    glGenVertexArrays(1, &m_debugVao);
    glGenBuffers(1, &m_debugVbo);
    glGenBuffers(1, &m_debugEbo);

    glBindVertexArray(m_debugVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_debugVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_debugEbo);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}

void Renderer::render(const Tissu::Solver& solver, const Camera& camera) {
    const auto& particles = solver.getParticles();
    if (particles.empty() || m_clothMeshes.empty())
        return;

    const size_t vertexCount = particles.size();

    m_normals.assign(vertexCount, Eigen::Vector3f::Zero());

    for (const auto& mesh : m_clothMeshes) {
        for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
            const unsigned int ia = mesh.indices[i];
            const unsigned int ib = mesh.indices[i + 1];
            const unsigned int ic = mesh.indices[i + 2];

            if (ia >= vertexCount || ib >= vertexCount || ic >= vertexCount)
                continue;

            const Eigen::Vector3d& pa = particles[ia].getPosition();
            const Eigen::Vector3d& pb = particles[ib].getPosition();
            const Eigen::Vector3d& pc = particles[ic].getPosition();

            Eigen::Vector3f faceNormal = (pb - pa).cross(pc - pa).cast<float>();

            m_normals[ia] += faceNormal;
            m_normals[ib] += faceNormal;
            m_normals[ic] += faceNormal;
        }
    }

    m_vertexBuffer.clear();
    m_vertexBuffer.reserve(vertexCount * 6);

    for (size_t i = 0; i < vertexCount; ++i) {
        const Eigen::Vector3d& pos = particles[i].getPosition();
        m_vertexBuffer.push_back(static_cast<float>(pos.x()));
        m_vertexBuffer.push_back(static_cast<float>(pos.y()));
        m_vertexBuffer.push_back(static_cast<float>(pos.z()));

        Eigen::Vector3f n = m_normals[i];
        float len = n.norm();
        if (len > 1e-6f)
            n /= len;

        m_vertexBuffer.push_back(n.x());
        m_vertexBuffer.push_back(n.y());
        m_vertexBuffer.push_back(n.z());
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size() * sizeof(float),
                 m_vertexBuffer.data(), GL_DYNAMIC_DRAW);

    m_shader.bind();

    Eigen::Matrix4f view = camera.getViewMatrix();
    Eigen::Matrix4f proj = camera.getProjectionMatrix();
    Eigen::Vector3f camPos = camera.getPosition();

    m_shader.setMat4("uView", view);
    m_shader.setMat4("uProjection", proj);
    m_shader.setVec3("uViewPos", camPos);

    static const Eigen::Vector3f lightDir =
        Eigen::Vector3f(1.0f, 2.0f, 1.5f).normalized();
    m_shader.setVec3("uLightDir", lightDir);

    glEnable(GL_DEPTH_TEST);

    for (const auto& mesh : m_clothMeshes) {
        if (mesh.indices.empty())
            continue;
        m_shader.setVec3(
            "COLOR_FRONT",
            Eigen::Vector3f(mesh.color[0], mesh.color[1], mesh.color[2]));
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()),
                       GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void Renderer::cleanup() {
    clearClothMeshes();
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
    if (m_ebo)
        glDeleteBuffers(1, &m_ebo);
    if (m_debugVao)
        glDeleteVertexArrays(1, &m_debugVao);
    if (m_debugVbo)
        glDeleteBuffers(1, &m_debugVbo);
    if (m_debugEbo)
        glDeleteBuffers(1, &m_debugEbo);
}

void Renderer::updateTopology() {
    if (m_clothMeshes.empty())
        return;

    m_clothMeshes[0].indices = m_indices;
    glBindVertexArray(m_clothMeshes[0].vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_clothMeshes[0].ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_indices.size() * sizeof(unsigned int), m_indices.data(),
                 GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void Renderer::updateColor(float* color) {
    if (!m_clothMeshes.empty()) {
        updateColor(0, color);
    }
}

void Renderer::updateColor(size_t index, float* color) {
    if (index < m_clothMeshes.size()) {
        m_clothMeshes[index].color[0] = color[0];
        m_clothMeshes[index].color[1] = color[1];
        m_clothMeshes[index].color[2] = color[2];
        m_clothMeshes[index].color[3] = color[3];
    }
}

void Renderer::clearClothMeshes() {
    for (auto& mesh : m_clothMeshes) {
        if (mesh.vao)
            glDeleteVertexArrays(1, &mesh.vao);
        if (mesh.ebo)
            glDeleteBuffers(1, &mesh.ebo);
    }
    m_clothMeshes.clear();
}

void Renderer::addClothMesh(const std::string& name,
                            const std::vector<unsigned int>& indices) {
    RenderMesh mesh;
    mesh.name = name;
    mesh.indices = indices;

    glGenVertexArrays(1, &mesh.vao);
    glGenBuffers(1, &mesh.ebo);

    glBindVertexArray(mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);

    constexpr GLsizei stride = 6 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Set a default color
    mesh.color[0] = 0.99f;
    mesh.color[1] = 0.96f;
    mesh.color[2] = 0.72f;
    mesh.color[3] = 1.0f;

    m_clothMeshes.push_back(mesh);
}

void Renderer::updateAmbient(float ambient) {
    m_shader.bind();
    m_shader.setFloat("AMBIENT", ambient);
}

void Renderer::updateDifusse(float difusse) {
    m_shader.bind();
    m_shader.setFloat("DIFFUSE", difusse);
}

void Renderer::updateSheenAmount(float sheenAmount) {
    m_shader.bind();
    m_shader.setFloat("SHEEN_AMOUNT", sheenAmount);
}

void Renderer::updateSheenWidth(float sheenWidth) {
    m_shader.bind();
    m_shader.setFloat("SHEEN_WIDTH", sheenWidth);
}

void Renderer::updateAnisotropy(float anisotropy) {
    m_shader.bind();
    m_shader.setFloat("ANISOTROPY", anisotropy);
}

void Renderer::updateAnisotropyWidth(float anisotropyWidth) {
    m_shader.bind();
    m_shader.setFloat("ANISOTROPY_WIDTH", anisotropyWidth);
}

static constexpr int SPHERE_STACKS = 10;
static constexpr int SPHERE_SLICES = 16;
static constexpr int PLANE_HALF_EXT = 10; // metres in each direction
static constexpr int PLANE_DIVS = 10;     // grid lines per side

void Renderer::buildSphereLines(const Eigen::Vector3d& center, double radius,
                                std::vector<float>& verts,
                                std::vector<unsigned int>& indices) {
    const int stacks = SPHERE_STACKS;
    const int slices = SPHERE_SLICES;
    unsigned int base = static_cast<unsigned int>(verts.size() / 3);

    for (int i = 0; i <= stacks; ++i) {
        double phi = M_PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {
            double theta = 2.0 * M_PI * j / slices;
            double x = radius * std::sin(phi) * std::cos(theta) + center.x();
            double y = radius * std::cos(phi) + center.y();
            double z = radius * std::sin(phi) * std::sin(theta) + center.z();
            verts.push_back(static_cast<float>(x));
            verts.push_back(static_cast<float>(y));
            verts.push_back(static_cast<float>(z));
        }
    }

    for (int i = 0; i <= stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int a = base + i * (slices + 1) + j;
            unsigned int b = base + i * (slices + 1) + j + 1;
            indices.push_back(a);
            indices.push_back(b);
        }
    }

    for (int j = 0; j <= slices; ++j) {
        for (int i = 0; i < stacks; ++i) {
            unsigned int a = base + i * (slices + 1) + j;
            unsigned int b = base + (i + 1) * (slices + 1) + j;
            indices.push_back(a);
            indices.push_back(b);
        }
    }
}

void Renderer::buildCapsuleLines(const Eigen::Vector3d& start,
                                 const Eigen::Vector3d& end, double radius,
                                 std::vector<float>& verts,
                                 std::vector<unsigned int>& indices) {
    const int slices = SPHERE_SLICES;
    const int hemiStacks = SPHERE_STACKS / 2;

    Eigen::Vector3d axis = end - start;
    double height = axis.norm();
    if (height < 1e-9) {
        buildSphereLines(0.5 * (start + end), radius, verts, indices);
        return;
    }
    Eigen::Vector3d axisN = axis / height;
    Eigen::Vector3d up(0, 1, 0);
    if (std::abs(axisN.dot(up)) > 0.99)
        up = Eigen::Vector3d(1, 0, 0);
    Eigen::Vector3d right = axisN.cross(up).normalized();
    Eigen::Vector3d fwd = axisN.cross(right).normalized();

    auto addPoint = [&](const Eigen::Vector3d& p) {
        verts.push_back(static_cast<float>(p.x()));
        verts.push_back(static_cast<float>(p.y()));
        verts.push_back(static_cast<float>(p.z()));
    };

    unsigned int base = static_cast<unsigned int>(verts.size() / 3);

    for (int i = 0; i <= hemiStacks; ++i) {
        double phi = M_PI * 0.5 * i / hemiStacks;
        for (int j = 0; j <= slices; ++j) {
            double theta = 2.0 * M_PI * j / slices;
            double rh = radius * std::cos(phi);
            double yOff = -radius * std::sin(phi);
            Eigen::Vector3d p =
                start + rh * (std::cos(theta) * right + std::sin(theta) * fwd) +
                yOff * axisN;
            addPoint(p);
        }
    }
    unsigned int bottomHemiCount = (hemiStacks + 1) * (slices + 1);

    for (int i = 0; i <= hemiStacks; ++i) {
        double phi = M_PI * 0.5 * i / hemiStacks;
        for (int j = 0; j <= slices; ++j) {
            double theta = 2.0 * M_PI * j / slices;
            double rh = radius * std::cos(phi);
            double yOff = radius * std::sin(phi);
            Eigen::Vector3d p =
                end + rh * (std::cos(theta) * right + std::sin(theta) * fwd) +
                yOff * axisN;
            addPoint(p);
        }
    }

    for (int i = 0; i <= hemiStacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int a = base + i * (slices + 1) + j;
            unsigned int b = base + i * (slices + 1) + j + 1;
            indices.push_back(a);
            indices.push_back(b);
        }
    }
    for (int j = 0; j <= slices; ++j) {
        for (int i = 0; i < hemiStacks; ++i) {
            unsigned int a = base + i * (slices + 1) + j;
            unsigned int b = base + (i + 1) * (slices + 1) + j;
            indices.push_back(a);
            indices.push_back(b);
        }
    }

    unsigned int topBase = base + bottomHemiCount;
    for (int i = 0; i <= hemiStacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int a = topBase + i * (slices + 1) + j;
            unsigned int b = topBase + i * (slices + 1) + j + 1;
            indices.push_back(a);
            indices.push_back(b);
        }
    }
    for (int j = 0; j <= slices; ++j) {
        for (int i = 0; i < hemiStacks; ++i) {
            unsigned int a = topBase + i * (slices + 1) + j;
            unsigned int b = topBase + (i + 1) * (slices + 1) + j;
            indices.push_back(a);
            indices.push_back(b);
        }
    }

    for (int j = 0; j < 4; ++j) {
        int slice = j * (slices / 4);
        unsigned int botEq = base + slice;
        unsigned int topEq = topBase + slice;
        indices.push_back(botEq);
        indices.push_back(topEq);
    }
}

void Renderer::buildPlaneLines(const Eigen::Vector3d& origin,
                               const Eigen::Vector3d& normal,
                               std::vector<float>& verts,
                               std::vector<unsigned int>& indices) {
    const int halfExt = PLANE_HALF_EXT;
    const int divs = PLANE_DIVS;

    Eigen::Vector3d n = normal.normalized();
    Eigen::Vector3d u(1, 0, 0);
    if (std::abs(n.dot(u)) > 0.99)
        u = Eigen::Vector3d(0, 1, 0);
    u = (u - u.dot(n) * n).normalized();
    Eigen::Vector3d v = n.cross(u);

    unsigned int base = static_cast<unsigned int>(verts.size() / 3);

    auto addPoint = [&](double s, double t) {
        Eigen::Vector3d p = origin + s * u + t * v;
        verts.push_back(static_cast<float>(p.x()));
        verts.push_back(static_cast<float>(p.y()));
        verts.push_back(static_cast<float>(p.z()));
    };

    double step = static_cast<double>(2 * halfExt) / divs;

    for (int i = 0; i <= divs; ++i) {
        double t = -halfExt + i * step;
        unsigned int idx =
            base + static_cast<unsigned int>(verts.size() / 3) - base;
        // Recalculate base-relative index each time
        unsigned int cur = static_cast<unsigned int>(verts.size() / 3);
        addPoint(-halfExt, t);
        addPoint(+halfExt, t);
        indices.push_back(cur);
        indices.push_back(cur + 1);
    }

    for (int i = 0; i <= divs; ++i) {
        double s = -halfExt + i * step;
        unsigned int cur = static_cast<unsigned int>(verts.size() / 3);
        addPoint(s, -halfExt);
        addPoint(s, +halfExt);
        indices.push_back(cur);
        indices.push_back(cur + 1);
    }

    {
        double arrowLen = halfExt * 0.2;
        unsigned int cur = static_cast<unsigned int>(verts.size() / 3);
        Eigen::Vector3d tip = origin + arrowLen * n;
        verts.push_back(static_cast<float>(origin.x()));
        verts.push_back(static_cast<float>(origin.y()));
        verts.push_back(static_cast<float>(origin.z()));
        verts.push_back(static_cast<float>(tip.x()));
        verts.push_back(static_cast<float>(tip.y()));
        verts.push_back(static_cast<float>(tip.z()));
        indices.push_back(cur);
        indices.push_back(cur + 1);
    }
}

void Renderer::renderColliders(const Tissu::World& world,
                               const Camera& camera) {
    if (!m_showColliders)
        return;

    const auto& colliders = world.getColliders();
    if (colliders.empty())
        return;

    std::vector<float> allVerts;
    std::vector<unsigned int> allIndices;

    struct DrawCall {
        unsigned int indexOffset;
        unsigned int indexCount;
        Eigen::Vector3f color;
    };
    std::vector<DrawCall> drawCalls;

    for (const auto& cPtr : colliders) {
        unsigned int idxOffset = static_cast<unsigned int>(allIndices.size());

        if (auto* s = dynamic_cast<const SphereCollider*>(cPtr.get())) {
            buildSphereLines(s->getCenter(), s->getRadius(), allVerts,
                             allIndices);
            drawCalls.push_back(
                {idxOffset,
                 static_cast<unsigned int>(allIndices.size() - idxOffset),
                 Eigen::Vector3f(0.30f, 0.60f, 1.00f)}); // blue

        } else if (auto* cap =
                       dynamic_cast<const CapsuleCollider*>(cPtr.get())) {
            buildCapsuleLines(cap->getStart(), cap->getEnd(), cap->getRadius(),
                              allVerts, allIndices);
            drawCalls.push_back(
                {idxOffset,
                 static_cast<unsigned int>(allIndices.size() - idxOffset),
                 Eigen::Vector3f(0.30f, 1.00f, 0.50f)}); // green

        } else if (auto* pl = dynamic_cast<const PlaneCollider*>(cPtr.get())) {
            buildPlaneLines(pl->getOrigin(), pl->getNormal(), allVerts,
                            allIndices);
            drawCalls.push_back(
                {idxOffset,
                 static_cast<unsigned int>(allIndices.size() - idxOffset),
                 Eigen::Vector3f(1.00f, 0.85f, 0.20f)}); // yellow

        } else if (auto* mc = dynamic_cast<const MeshCollider*>(cPtr.get())) {
            // Use already-transformed world vertices
            const auto& wverts = mc->getWorldVertices();
            const auto& tris = mc->getTriangles();
            if (wverts.empty() || tris.empty())
                continue;

            unsigned int base = static_cast<unsigned int>(allVerts.size() / 3);

            for (const auto& wv : wverts) {
                allVerts.push_back(static_cast<float>(wv.x()));
                allVerts.push_back(static_cast<float>(wv.y()));
                allVerts.push_back(static_cast<float>(wv.z()));
            }

            for (const auto& tri : tris) {
                unsigned int a = base + static_cast<unsigned int>(tri.a);
                unsigned int b = base + static_cast<unsigned int>(tri.b);
                unsigned int c = base + static_cast<unsigned int>(tri.c);
                allIndices.push_back(a);
                allIndices.push_back(b);
                allIndices.push_back(b);
                allIndices.push_back(c);
                allIndices.push_back(c);
                allIndices.push_back(a);
            }
            drawCalls.push_back(
                {idxOffset,
                 static_cast<unsigned int>(allIndices.size() - idxOffset),
                 Eigen::Vector3f(1.00f, 0.50f, 0.20f)}); // orange
        }
    }

    if (allVerts.empty() || allIndices.empty())
        return;

    glBindVertexArray(m_debugVao);

    glBindBuffer(GL_ARRAY_BUFFER, m_debugVbo);
    glBufferData(GL_ARRAY_BUFFER, allVerts.size() * sizeof(float),
                 allVerts.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_debugEbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 allIndices.size() * sizeof(unsigned int), allIndices.data(),
                 GL_DYNAMIC_DRAW);

    m_colliderShader.bind();
    m_colliderShader.setMat4("uView", camera.getViewMatrix());
    m_colliderShader.setMat4("uProjection", camera.getProjectionMatrix());

    glEnable(GL_DEPTH_TEST);

    for (const auto& dc : drawCalls) {
        m_colliderShader.setVec3("uColor", dc.color);
        glDrawElements(GL_LINES, static_cast<GLsizei>(dc.indexCount),
                       GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(dc.indexOffset *
                                                     sizeof(unsigned int)));
    }

    glBindVertexArray(0);
}

} // namespace Viewer
} // namespace Tissu