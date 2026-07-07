#include "data-structures/BVH.hpp"

namespace Tissu {

void BVH::build(const std::vector<Eigen::Vector3d>& vertices,
                const std::vector<Triangle>& triangles) {
  if (triangles.empty()) return;

  m_triangles = triangles;
  m_nodes.clear();
  m_nodes.reserve(triangles.size() * 2);
  m_rootIndex = buildRecursive(m_triangles, vertices, 0, m_triangles.size());
}

int BVH::buildRecursive(std::vector<Triangle>& tempTriangles,
                        const std::vector<Eigen::Vector3d>& vertices, int start,
                        int end) {
  int nodeIdx = m_nodes.size();
  m_nodes.emplace_back();
  int numTriangles = end - start;

  // Leaf
  if (numTriangles <= LEAF_SIZE) {
    m_nodes[nodeIdx].triangleIndex = start;
    m_nodes[nodeIdx].primitiveCount = numTriangles;
    for (int i = start; i < end; ++i) {
      const Triangle& tri = tempTriangles[i];
      m_nodes[nodeIdx].bbox.extend(vertices[tri.a]);
      m_nodes[nodeIdx].bbox.extend(vertices[tri.b]);
      m_nodes[nodeIdx].bbox.extend(vertices[tri.c]);
    }
    return nodeIdx;
  }

  Eigen::AlignedBox3d centroidBounds;
  for (int idx = start; idx < end; ++idx) {
    const Triangle& tri = tempTriangles[idx];
    Eigen::Vector3d centroid =
        (vertices[tri.a] + vertices[tri.b] + vertices[tri.c]) / 3.0;
    centroidBounds.extend(centroid);
  }

  // (0 = X   1 = Y   2 = Z)
  int longestAxis = 0;
  Eigen::Vector3d diagonal = centroidBounds.diagonal();
  if (diagonal.y() > diagonal.x() && diagonal.y() > diagonal.z())
    longestAxis = 1;
  else if (diagonal.z() > diagonal.x() && diagonal.z() > diagonal.y())
    longestAxis = 2;

  int mid = start + numTriangles / 2;
  std::nth_element(
      tempTriangles.begin() + start, tempTriangles.begin() + mid,
      tempTriangles.begin() + end,
      [&vertices, longestAxis](const Triangle& t1, const Triangle& t2) {
        Eigen::Vector3d c1 =
            (vertices[t1.a] + vertices[t1.b] + vertices[t1.c]) / 3.0;
        Eigen::Vector3d c2 =
            (vertices[t2.a] + vertices[t2.b] + vertices[t2.c]) / 3.0;
        return c1[longestAxis] < c2[longestAxis];
      });

  m_nodes[nodeIdx].left = buildRecursive(tempTriangles, vertices, start, mid);
  m_nodes[nodeIdx].right = buildRecursive(tempTriangles, vertices, mid, end);
  m_nodes[nodeIdx].bbox.extend(m_nodes[m_nodes[nodeIdx].left].bbox);
  m_nodes[nodeIdx].bbox.extend(m_nodes[m_nodes[nodeIdx].right].bbox);

  return nodeIdx;
}

void BVH::query(const Eigen::Vector3d& point, double radius,
                std::vector<int>& outTriangles) const {
  outTriangles.clear();
  if (m_rootIndex == -1 || m_nodes.empty()) return;

  queryRecursive(m_rootIndex, point, radius * radius, outTriangles);
}

void BVH::queryRecursive(int nodeIdx, const Eigen::Vector3d& point,
                         double squaredRadius,
                         std::vector<int>& outTriangles) const {
  const BVHNode& node = m_nodes[nodeIdx];

  if (node.bbox.squaredExteriorDistance(point) > squaredRadius) return;

  if (node.isLeaf()) {
    for (int i = 0; i < node.primitiveCount; ++i) {
      outTriangles.push_back(node.triangleIndex + i);
    }
  } else {
    double dLeft = m_nodes[node.left].bbox.squaredExteriorDistance(point);
    double dRight = m_nodes[node.right].bbox.squaredExteriorDistance(point);

    if (dLeft < dRight) {
      queryRecursive(node.left, point, squaredRadius, outTriangles);
      queryRecursive(node.right, point, squaredRadius, outTriangles);
    } else {
      queryRecursive(node.right, point, squaredRadius, outTriangles);
      queryRecursive(node.left, point, squaredRadius, outTriangles);
    }
  }
}

int BVH::closestTriangle(
    const Eigen::Vector3d& point,
    const std::vector<Eigen::Vector3d>& vertices) const {
  if (m_rootIndex == -1 || m_nodes.empty()) return -1;
  double bestDistSq = std::numeric_limits<double>::max();
  return closestRecursive(m_rootIndex, point, vertices, bestDistSq);
}

int BVH::closestRecursive(int nodeIdx, const Eigen::Vector3d& point,
                          const std::vector<Eigen::Vector3d>& vertices,
                          double& bestDistSq) const {
  const BVHNode& node = m_nodes[nodeIdx];

  double distSq = node.bbox.squaredExteriorDistance(point);
  if (distSq > bestDistSq) return -1;

  if (node.isLeaf()) {
    int bestTri = -1;
    for (int i = 0; i < node.primitiveCount; ++i) {
      int triIdx = node.triangleIndex + i;
      const Triangle& tri = m_triangles[triIdx];
      Eigen::Vector3d closest =
          closestPointOnTriangle(point, vertices[tri.a], vertices[tri.b],
                                 vertices[tri.c]);
      double dSq = (point - closest).squaredNorm();
      if (dSq < bestDistSq) {
        bestDistSq = dSq;
        bestTri = triIdx;
      }
    }
    return bestTri;
  }

  int first = node.left;
  int second = node.right;

  double dFirst = m_nodes[node.left].bbox.squaredExteriorDistance(point);
  double dSecond = m_nodes[node.right].bbox.squaredExteriorDistance(point);

  if (dFirst > dSecond) {
    std::swap(first, second);
    std::swap(dFirst, dSecond);
  }

  int result = closestRecursive(first, point, vertices, bestDistSq);
  if (dSecond > bestDistSq) {
    return result;
  }

  int result2 = closestRecursive(second, point, vertices, bestDistSq);
  return result2 != -1 ? result2 : result;
}

}  // namespace Tissu