#include "gtest/gtest.h"
#include "math/Geometry.hpp"
#include "Eigen/Dense"

using namespace Tissu;

TEST(GeometryTest, ClosestPointOnTriangle) {
  Eigen::Vector3d a(0, 0, 0);
  Eigen::Vector3d b(1, 0, 0);
  Eigen::Vector3d c(0, 1, 0);

  // Point above the triangle
  Eigen::Vector3d p1(0.25, 0.25, 1);
  Eigen::Vector3d closest1 = closestPointOnTriangle(p1, a, b, c);
  EXPECT_NEAR(closest1.x(), 0.25, 1e-9);
  EXPECT_NEAR(closest1.y(), 0.25, 1e-9);
  EXPECT_NEAR(closest1.z(), 0.0, 1e-9);

  // Point below the triangle
  Eigen::Vector3d p2(0.25, 0.25, -1);
  Eigen::Vector3d closest2 = closestPointOnTriangle(p2, a, b, c);
  EXPECT_NEAR(closest2.x(), 0.25, 1e-9);
  EXPECT_NEAR(closest2.y(), 0.25, 1e-9);
  EXPECT_NEAR(closest2.z(), 0.0, 1e-9);

  // Edge
  Eigen::Vector3d p3(1.0, 0.0, 0.0);
  Eigen::Vector3d closest3 = closestPointOnTriangle(p3, a, b, c);
  EXPECT_NEAR(closest3.x(), 1.0, 1e-9);
  EXPECT_NEAR(closest3.y(), 0.0, 1e-9);
  EXPECT_NEAR(closest3.z(), 0.0, 1e-9);

  // Centroid
  Eigen::Vector3d p4(1.0/3.0, 1.0/3.0, 1.0/3.0);
  Eigen::Vector3d closest4 = closestPointOnTriangle(p4, a, b, c);
  EXPECT_NEAR(closest4.x(), 1.0/3.0, 1e-9);
  EXPECT_NEAR(closest4.y(), 1.0/3.0, 1e-9);
  EXPECT_NEAR(closest4.z(), 0.0, 1e-9);
}