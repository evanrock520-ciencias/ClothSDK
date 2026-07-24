#include "Eigen/Dense"
#include "math/Geometry.hpp"
#include "gtest/gtest.h"

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
    Eigen::Vector3d p4(1.0 / 3.0, 1.0 / 3.0, 1.0 / 3.0);
    Eigen::Vector3d closest4 = closestPointOnTriangle(p4, a, b, c);
    EXPECT_NEAR(closest4.x(), 1.0 / 3.0, 1e-9);
    EXPECT_NEAR(closest4.y(), 1.0 / 3.0, 1e-9);
    EXPECT_NEAR(closest4.z(), 0.0, 1e-9);
}

TEST(GeometryTest, TriangleSegmentIntersection) {
    const Eigen::Vector3d a(0, 0, 0);
    const Eigen::Vector3d b(1, 0, 0);
    const Eigen::Vector3d c(0, 1, 0);

    // Segment intersecting the triangle
    const Eigen::Vector3d p1(0.25, 0.25, -1);
    const Eigen::Vector3d p2(0.25, 0.25, 1);
    SegmentTriangleHit hit1 = intersectSegmentTriangle(p1, p2, a, b, c);
    EXPECT_TRUE(hit1.hit);
    EXPECT_NEAR(hit1.point.x(), 0.25, 1e-9);
    EXPECT_NEAR(hit1.point.y(), 0.25, 1e-9);
    EXPECT_NEAR(hit1.point.z(), 0.0, 1e-9);

    // Vertex
    const Eigen::Vector3d p3(1.0, 0.0, -1.0);
    const Eigen::Vector3d p4(1.0, 0.0, 1.0);
    SegmentTriangleHit hit2 = intersectSegmentTriangle(p3, p4, a, b, c);
    EXPECT_TRUE(hit2.hit);

    // Edge
    const Eigen::Vector3d p5(0.0, 0.5, -1.0);
    const Eigen::Vector3d p6(1.0, 0.0, 1.0);
    SegmentTriangleHit hit3 = intersectSegmentTriangle(p5, p6, a, b, c);
    EXPECT_TRUE(hit3.hit);

    // Segment not intersecting the triangle
    const Eigen::Vector3d p7(2.0, 2.0, -1);
    const Eigen::Vector3d p8(2.0, 2.0, 1);
    const SegmentTriangleHit hit4 = intersectSegmentTriangle(p7, p8, a, b, c);
    EXPECT_FALSE(hit4.hit);
}

TEST(GeometryTest, EdgeEdgeXShapeCross) {
    // X shape
    const Eigen::Vector3d p1(0, 1, 0);
    const Eigen::Vector3d p2(1, 0, 0);
    const Eigen::Vector3d p3(1, 1, 0);
    const Eigen::Vector3d p4(0, 0, 0);

    EdgeEdgeHit hit = closestPointsEdgeEdge(p1, p2, p3, p4, 0.0);
    EXPECT_TRUE(hit.hit);
    EXPECT_NEAR(hit.s, 0.5, 1e-9);
    EXPECT_NEAR(hit.t, 0.5, 1e-9);
    EXPECT_NEAR(hit.distance, 0.0, 1e-9);
}

TEST(GeometryTest, EdgeEdgeSegmentIsPoint) {
    // Segment 2 is a point
    const Eigen::Vector3d p5(0, 0, 0);
    const Eigen::Vector3d p6(0, 1, 0);
    const Eigen::Vector3d p7(1, 0, 0);

    const EdgeEdgeHit hit2 = closestPointsEdgeEdge(p5, p6, p7, p7, 0.0);
    EXPECT_FALSE(hit2.hit);
    EXPECT_NEAR(hit2.distance, 1.0, 1e-9);

    // Inside the first segment
    const Eigen::Vector3d p8(0, 0.5, 0);
    const EdgeEdgeHit hit3 = closestPointsEdgeEdge(p5, p6, p8, p8, 0.0);
    EXPECT_TRUE(hit3.hit);
    EXPECT_NEAR(hit3.distance, 0.0, 1e-9);
}

TEST(GeometryTest, EdgeEdgeParallelSegments) {
    // Parallel Segments
    const Eigen::Vector3d p8(0, 0, 0);
    const Eigen::Vector3d p9(1, 0, 0);
    const Eigen::Vector3d p10(0, 1, 0);
    const Eigen::Vector3d p11(1, 1, 0);

    const EdgeEdgeHit hit3 = closestPointsEdgeEdge(p8, p9, p10, p11, 0.0);
    EXPECT_FALSE(hit3.hit);
    EXPECT_NEAR(hit3.distance, 1.0, 1e-9);
}

TEST(GeometryTest, EdgeEdgeSkewLines3D) {
    // Skew Lines
    const Eigen::Vector3d p12(0, 0, 0);
    const Eigen::Vector3d p13(1, 0, 0);
    const Eigen::Vector3d p14(0, 0, 2);
    const Eigen::Vector3d p15(1, 0, 2);

    const EdgeEdgeHit hit4 = closestPointsEdgeEdge(p12, p13, p14, p15, 0.0);
    EXPECT_FALSE(hit4.hit);
    EXPECT_NEAR(hit4.distance, 2.0, 1e-9);
}

TEST(GeometryTest, EdgeEdgeThicknessMargin) {
    const Eigen::Vector3d p1(0, 0, 0);
    const Eigen::Vector3d p2(2, 0, 0);
    const Eigen::Vector3d q1(1, 0.03, 0);
    const Eigen::Vector3d q2(1, 2.0, 0);

    const EdgeEdgeHit hitSmall = closestPointsEdgeEdge(p1, p2, q1, q2, 0.01);
    EXPECT_FALSE(hitSmall.hit);

    const EdgeEdgeHit hitLarge = closestPointsEdgeEdge(p1, p2, q1, q2, 0.05);
    EXPECT_TRUE(hitLarge.hit);
}

TEST(GeometryTest, EdgeEdgeSharedVertex) {
    // Both edges start at the same origin vertex (0, 0, 0)
    const Eigen::Vector3d origin(0, 0, 0);
    const Eigen::Vector3d p2(1, 0, 0);
    const Eigen::Vector3d q2(0, 1, 0);

    const EdgeEdgeHit hit = closestPointsEdgeEdge(origin, p2, origin, q2, 0.0);
    EXPECT_NEAR(hit.s, 0.0, 1e-9);
    EXPECT_NEAR(hit.t, 0.0, 1e-9);
    EXPECT_NEAR(hit.distance, 0.0, 1e-9);
    EXPECT_TRUE(hit.hit);
}
