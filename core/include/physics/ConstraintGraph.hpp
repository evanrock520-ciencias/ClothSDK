#pragma once

#include "physics/Constraint.hpp"
#include <memory>
#include <vector>
namespace Tissu {

class ConstraintGraph {
public:
    ConstraintGraph();

    void buildFrom(const std::vector<std::unique_ptr<Constraint>>& constraints);

    inline int nodeCount() const { return m_nodeCount; }
    inline int edgeCount() const { return m_edgeCount; }

    const std::vector<int>& neighbors(int node) const;
    bool hasEdge(int i, int j) const;

private:
    int m_nodeCount = 0;
    int m_edgeCount = 0;
    std::vector<std::vector<int>> m_adjacency;
};

}