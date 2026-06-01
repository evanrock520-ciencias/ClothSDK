#include "physics/ConstraintGraph.hpp"
#include "utils/Logger.hpp"

namespace Tissu {

ConstraintGraph::ConstraintGraph() {}

void ConstraintGraph::buildFrom(const std::vector<std::unique_ptr<Constraint>>& constraints) {
    m_nodeCount = static_cast<int>(constraints.size());
    m_edgeCount = 0;
    m_adjacency.assign(m_nodeCount, {});

    std::unordered_map<int, std::vector<int>> particleToConstraints;

    for (int idx = 0; idx < m_nodeCount; idx++) {
        for (int pid : constraints[idx]->getParticleIds()) {
            particleToConstraints[pid].push_back(idx);
        }
    }

    for (const auto& [pid, constraintList] : particleToConstraints) {
        for (int a = 0; a < (int)constraintList.size(); a++) {
            for (int b = a + 1; b < (int)constraintList.size(); b++) {
                int idx = constraintList[a];
                int jdx = constraintList[b];

                auto& ni = m_adjacency[idx];
                if (std::find(ni.begin(), ni.end(), jdx) == ni.end()) {
                    ni.push_back(jdx);
                    m_adjacency[jdx].push_back(idx);
                    m_edgeCount++;
                }
            }
        }
    }
}

}