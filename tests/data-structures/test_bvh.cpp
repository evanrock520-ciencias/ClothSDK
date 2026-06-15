#include <vector>

#include "data-structures/BVH.hpp"
#include "gtest/gtest.h"
#include "math/Types.hpp"
#include "Eigen/Dense"

using namespace Tissu;

TEST(BVHTest, EmptyBVHQueryReturnsNothing) {
  BVH bvh;
  bvh.build({}, {});
  std::vector<int> result;
  bvh.query({0, 0, 0}, 1.0, result);
  EXPECT_TRUE(result.empty());
}

TEST(BVHTest, EmptyBVHClosestTriangleReturnsMinusOne) {
  BVH bvh;
  bvh.build({}, {});
  std::vector<Eigen::Vector3d> verts;
  EXPECT_EQ(-1, bvh.closestTriangle({0, 0, 0}, verts));
}

TEST(BVHTest, SingleTriangleQueryAtCentroid) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0}
  };
  std::vector<Triangle> tris = {{0, 1, 2}};

  BVH bvh;
  bvh.build(verts, tris);

  std::vector<int> result;
  bvh.query({0.25, 0.25, 0}, 0.5, result);
  ASSERT_EQ(1, result.size());
  EXPECT_EQ(0, result[0]);
}

TEST(BVHTest, SingleTriangleClosestReturnsExactTriangle) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0}
  };
  std::vector<Triangle> tris = {{0, 1, 2}};

  BVH bvh;
  bvh.build(verts, tris);

  int idx = bvh.closestTriangle({0.25, 0.25, 0}, verts);
  EXPECT_EQ(0, idx);
}


TEST(BVHTest, QueryFindsMultipleTriangles) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0}, {1, 0, 0}, {0, 1, 0},
    {1, 1, 0}, {1, 0, 0}, {0, 1, 0}
  };
  std::vector<Triangle> tris = {{0, 1, 2}, {3, 4, 5}};

  BVH bvh;
  bvh.build(verts, tris);

  std::vector<int> result;
  bvh.query({0.5, 0.5, 0}, 1.0, result);
  EXPECT_EQ(2, result.size());
}

TEST(BVHTest, QueryEmptyWhenFarAway) {
  std::vector<Eigen::Vector3d> verts = {
    {100, 100, 100}, {101, 100, 100}, {100, 101, 100}
  };
  std::vector<Triangle> tris = {{0, 1, 2}};

  BVH bvh;
  bvh.build(verts, tris);

  std::vector<int> result;
  bvh.query({0, 0, 0}, 1.0, result);
  EXPECT_TRUE(result.empty());
}

TEST(BVHTest, QueryHugeRadiusFindsAll) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0},   {1, 0, 0},  {0, 1, 0},
    {10, 10, 0}, {11, 10, 0}, {10, 11, 0},
  };
  std::vector<Triangle> tris = {{0, 1, 2}, {3, 4, 5}};

  BVH bvh;
  bvh.build(verts, tris);

  std::vector<int> result;
  bvh.query({0, 0, 0}, 100.0, result);
  EXPECT_EQ(2, result.size());
}

TEST(BVHTest, ClosestPicksNearestAmongMany) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0},   {1, 0, 0},   {0, 1, 0},      // tri 0 – centroid (0.333, 0.333, 0)
    {10, 10, 0}, {11, 10, 0}, {10, 11, 0},    // tri 1 – far
    {5, 0, 0},   {6, 0, 0},   {5, 1, 0},       // tri 2 – medium
  };
  std::vector<Triangle> tris = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}};

  BVH bvh;
  bvh.build(verts, tris);

  // point near tri 0
  int idx1 = bvh.closestTriangle({0.3, 0.3, 0}, verts);
  EXPECT_EQ(0, idx1);

  int idx2 = bvh.closestTriangle({10.2, 10.2, 0.0}, verts);
  EXPECT_EQ(1, idx2);
}

TEST(BVHTest, HandlesMultipleTrianglesInLeaf) {
  std::vector<Eigen::Vector3d> verts;
  std::vector<Triangle> tris;
  for (int i = 0; i < 6; ++i) {
    double x = i * 0.01;
    verts.push_back({x, 0, 0});
    verts.push_back({x + 0.5, 0, 0});
    verts.push_back({x, 0.5, 0});
    tris.push_back({3 * i, 3 * i + 1, 3 * i + 2});
  }

  BVH bvh;
  bvh.build(verts, tris);

  std::vector<int> result;
  bvh.query({0.0, 0.1, 0}, 1.0, result);
  EXPECT_EQ(6, result.size());

  int closest = bvh.closestTriangle({0.0, 0.1, 0}, verts);
  EXPECT_GE(closest, 0);
  EXPECT_LT(closest, 6);
}

TEST(BVHTest, QueryTinyRadiusMatchesClosestTriangle) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0},   {1, 0, 0},   {0, 1, 0},
    {5, 0, 0},   {6, 0, 0},   {5, 1, 0},
  };
  std::vector<Triangle> tris = {{0, 1, 2}, {3, 4, 5}};

  BVH bvh;
  bvh.build(verts, tris);

  Eigen::Vector3d queryPt(0.25, 0.25, 0);

  std::vector<int> queryResult;
  bvh.query(queryPt, 1e-6, queryResult);

  int closestIdx = bvh.closestTriangle(queryPt, verts);

  if (!queryResult.empty()) {
    EXPECT_EQ(closestIdx, queryResult[0]);
  }
}

TEST(BVHTest, LargeSceneDoesNotCrash) {
  const int N = 1000;

  std::vector<Eigen::Vector3d> verts;
  std::vector<Triangle> tris;
  verts.reserve(3 * N);
  tris.reserve(N);

  for (int i = 0; i < N; ++i) {
    double x = (i % 50) * 2.0;
    double y = (i / 50) * 2.0;
    verts.push_back({x, y, 0});
    verts.push_back({x + 1, y, 0});
    verts.push_back({x, y + 1, 0});
    tris.push_back({3 * i, 3 * i + 1, 3 * i + 2});
  }

  BVH bvh;
  bvh.build(verts, tris);

  std::vector<int> result;
  bvh.query({0, 0, 0}, 1.5, result);
  EXPECT_GT(result.size(), 0);

  int closest = bvh.closestTriangle({0, 0, 0}, verts);
  EXPECT_GE(closest, 0);
  EXPECT_LT(closest, N);
}

TEST(BVHTest, ClosestZeroDistanceOnSurface) {
  std::vector<Eigen::Vector3d> verts = {
    {0, 0, 0}, {2, 0, 0}, {0, 2, 0}
  };
  std::vector<Triangle> tris = {{0, 1, 2}};

  BVH bvh;
  bvh.build(verts, tris);

  int idx = bvh.closestTriangle({0.5, 0.5, 0}, verts);
  EXPECT_EQ(0, idx);
}
