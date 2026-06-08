#include "physics/BendingConstraint.hpp"
#include "physics/Constraint.hpp"
#include "physics/ConstraintGraph.hpp"
#include "physics/DistanceConstraint.hpp"
#include "physics/PinConstraint.hpp"
#include "physics/VolumeConstraint.hpp"
#include <Eigen/Dense>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using namespace Tissu;

static std::vector<Particle> makeParticles(int count) {
  std::vector<Particle> particles;
  for (int i = 0; i < count; i++)
    particles.emplace_back(Eigen::Vector3d(i * 1.0, 0.0, 0.0));
  return particles;
}

TEST(ConstraintGraph, EmptyConstraintsProduceEmptyGraph) {
  auto particles = makeParticles(20);
  std::vector<std::unique_ptr<Constraint>> constraints;
  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 0);
  EXPECT_EQ(graph.edgeCount(), 0);
}

TEST(ConstraintGraph, SingleConstraintProducesOneNodeNoEdges) {
  auto particles = makeParticles(2);
  std::vector<std::unique_ptr<Constraint>> constraints;
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 1, 1.0, 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 1);
  EXPECT_EQ(graph.edgeCount(), 0);
}

TEST(ConstraintGraph, DisjointConstraintsHaveNoEdge) {
  auto particles = makeParticles(4);
  std::vector<std::unique_ptr<Constraint>> constraints;
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 1, 1.0, 0.0));
  constraints.push_back(std::make_unique<DistanceConstraint>(2, 3, 1.0, 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 2);
  EXPECT_EQ(graph.edgeCount(), 0);
}

TEST(ConstraintGraph, SharedParticleCreatesEdge) {
  auto particles = makeParticles(3);
  std::vector<std::unique_ptr<Constraint>> constraints;
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 1, 1.0, 0.0));
  constraints.push_back(std::make_unique<DistanceConstraint>(1, 2, 1.0, 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 2);
  EXPECT_EQ(graph.edgeCount(), 1);
}

TEST(ConstraintGraph, BendingAndDistanceSharingParticlesAreAdjacent) {
  auto particles = makeParticles(6);
  std::vector<std::unique_ptr<Constraint>> constraints;
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 1, 1.0, 0.0));
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 2, 1.0, 0.0));
  constraints.push_back(std::make_unique<DistanceConstraint>(1, 3, 1.0, 0.0));
  constraints.push_back(
      std::make_unique<BendingConstraint>(1, 2, 3, 4, 1.0, 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 4);
  EXPECT_EQ(graph.edgeCount(), 5);
}

TEST(ConstraintGraph, PinAndDistanceSharingParticleAreAdjacent) {
  auto particles = makeParticles(2);
  std::vector<std::unique_ptr<Constraint>> constraints;
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 1, 1.0, 0.0));
  constraints.push_back(
      std::make_unique<PinConstraint>(1, particles[1].getPosition(), 0.0));
  constraints.push_back(
      std::make_unique<PinConstraint>(0, particles[0].getPosition(), 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 3);
  EXPECT_EQ(graph.edgeCount(), 2);
}

TEST(ConstraintGraph, VolumeConstraintIsIsolated) {
  auto particles = makeParticles(10);
  std::vector<std::unique_ptr<Constraint>> constraints;
  std::vector<Triangle> triangles;
  constraints.push_back(
      std::make_unique<VolumeConstraint>(triangles, particles, 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  EXPECT_EQ(graph.nodeCount(), 1);
  EXPECT_EQ(graph.edgeCount(), 0);
}

TEST(ConstraintGraph, WeightInRange) {
  auto particles = makeParticles(10);
  std::vector<std::unique_ptr<Constraint>> constraints;

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  for (int idx = 0; idx < graph.nodeCount(); idx++) {
    double weight = graph.getWeight(idx);
    EXPECT_TRUE(weight >= 0 && weight <= 1.0);
  }
}

TEST(ConstraintGraph, EmptyGraphTakesOneColor) {
  auto particles = makeParticles(10);
  std::vector<std::unique_ptr<Constraint>> constraints;

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  auto colors = graph.colorBatches();
  EXPECT_EQ(colors.size(), 1);
}

TEST(ConstraintGraph, K3GraphTakesThreeColors) {
  auto particles = makeParticles(3);
  std::vector<std::unique_ptr<Constraint>> constraints;
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 1, 1.0, 0.0));
  constraints.push_back(std::make_unique<DistanceConstraint>(1, 2, 1.0, 0.0));
  constraints.push_back(std::make_unique<DistanceConstraint>(0, 2, 1.0, 0.0));

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  auto colors = graph.colorBatches();
  EXPECT_EQ(colors.size(), 3);
}

TEST(ConstraintGraph, EveryBatchIsAnIndependentSet) {
  int size = 20;
  auto particles = makeParticles(size);
  std::vector<std::unique_ptr<Constraint>> constraints;
  for (int idx = 0; idx < size - 3; idx++) {
    constraints.push_back(
        std::make_unique<DistanceConstraint>(idx, idx + 1, 1.0, 0.0));
    constraints.push_back(
        std::make_unique<DistanceConstraint>(idx, idx + 2, 1.0, 0.0));
    constraints.push_back(
        std::make_unique<DistanceConstraint>(idx, idx + 3, 1.0, 0.0));
  }

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  auto batches = graph.colorBatches();
  for (const auto &batch : batches) {
    for (size_t i = 0; i < batch.size(); ++i) {
      for (size_t j = i + 1; j < batch.size(); ++j) {
        EXPECT_FALSE(graph.hasEdge(batch[i], batch[j]));
      }
    }
  }
}

TEST(ConstraintGraph, NodesInBatchesEqualsTotalNodes) {
  int size = 20;
  auto particles = makeParticles(size);
  std::vector<std::unique_ptr<Constraint>> constraints;
  for (int idx = 0; idx < size - 3; idx++) {
    constraints.push_back(
        std::make_unique<DistanceConstraint>(idx, idx + 1, 1.0, 0.0));
    constraints.push_back(
        std::make_unique<DistanceConstraint>(idx, idx + 2, 1.0, 0.0));
    constraints.push_back(
        std::make_unique<DistanceConstraint>(idx, idx + 3, 1.0, 0.0));
  }

  ConstraintGraph graph;
  graph.buildFrom(constraints, 42);

  int nodeCount = 0;

  auto batches = graph.colorBatches();
  for (const auto &batch : batches) {
    for (size_t i = 0; i < batch.size(); ++i)
      nodeCount++;
  }

  EXPECT_EQ(nodeCount, graph.nodeCount());
}