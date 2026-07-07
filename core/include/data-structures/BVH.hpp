#pragma once

#include <limits>
#include <vector>
#include <Eigen/Dense>
#include "math/Geometry.hpp"
#include "math/Types.hpp"

namespace Tissu {

struct BVHNode {
    Eigen::AlignedBox3d bbox;

    int left = -1;
    int right = -1;
    int triangleIndex = -1;
    int primitiveCount = 1;

    BVHNode() = default;

    bool isLeaf() const {
        return left == -1 && right == -1;
    }

    void calculateLeafBox(const Triangle& tri, const std::vector<Eigen::Vector3d>& vertices) {
        bbox.extend(vertices[tri.a]);
        bbox.extend(vertices[tri.b]);
        bbox.extend(vertices[tri.c]);
    }
};

class BVH {
public:
    static constexpr int LEAF_SIZE = 4;

    BVH() = default;

    void build(const std::vector<Eigen::Vector3d>& vertices,
               const std::vector<Triangle>& triangles);
    void query(const Eigen::Vector3d& point, double radius,
               std::vector<int>& outTriangles) const;
    int closestTriangle(const Eigen::Vector3d& point,
                        const std::vector<Eigen::Vector3d>& vertices) const;
    const Triangle& getTriangle(int index) const { return m_triangles[index]; }

private:
    int buildRecursive(std::vector<Triangle>& tempTriangles,
                       const std::vector<Eigen::Vector3d>& vertices,
                       int start, int end);

    void queryRecursive(int nodeIdx, const Eigen::Vector3d& point,
                        double squaredRadius,
                        std::vector<int>& outTriangles) const;
    int closestRecursive(int nodeIdx, const Eigen::Vector3d& point,
                         const std::vector<Eigen::Vector3d>& vertices,
                         double& bestDistSq) const;

    std::vector<BVHNode> m_nodes;
    std::vector<Triangle> m_triangles;
    int m_rootIndex = -1;
};

} // namespace Tissu