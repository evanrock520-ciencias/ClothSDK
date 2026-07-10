#pragma once

#include <memory>
#include <vector>

#include "physics/Constraint.hpp"
namespace Tissu {

class ConstraintGraph {
public:
    ConstraintGraph();

    void buildFrom(const std::vector<std::unique_ptr<Constraint>>& constraints,
                   unsigned int seed);

    inline int nodeCount() const { return m_nodeCount; }
    inline int edgeCount() const { return m_edgeCount; }

    const std::vector<int>& neighbors(int node) const {
        return m_adjacency[node];
    }
    double getWeight(int node) const { return m_weights[node]; }
    bool hasEdge(int i, int j) const {
        const auto& neighbors = m_adjacency[i];
        return std::find(neighbors.begin(), neighbors.end(), j) !=
               neighbors.end();
    };

    std::vector<std::vector<int>> colorBatches();

private:
    int m_nodeCount = 0;
    int m_edgeCount = 0;
    std::vector<std::vector<int>> m_adjacency;
    std::vector<double> m_weights;
};

} // namespace Tissu