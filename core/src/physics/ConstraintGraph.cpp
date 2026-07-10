#include "physics/ConstraintGraph.hpp"

#include <cstddef>
#include <random>
#include <vector>

#include "utils/Logger.hpp"

namespace Tissu {

ConstraintGraph::ConstraintGraph() {}

void ConstraintGraph::buildFrom(
    const std::vector<std::unique_ptr<Constraint>>& constraints,
    unsigned int seed = 42) {
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

    m_weights.resize(m_nodeCount);
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    for (int i = 0; i < m_nodeCount; i++)
        m_weights[i] = dist(rng);
}

std::vector<std::vector<int>> ConstraintGraph::colorBatches() {
    if (m_nodeCount == 0)
        return {{}};

    std::vector<int> color(m_nodeCount, -1);
    std::vector<char> done(m_nodeCount, 0);
    std::vector<std::vector<int>> batches;
    int processedNodes = 0;

    while (processedNodes < m_nodeCount) {
        std::vector<int> currentBatch;

#pragma omp parallel
        {
            std::vector<int> localBatch;
#pragma omp for
            for (int idx = 0; idx < m_nodeCount; idx++) {
                if (done[idx])
                    continue;

                bool wins = true;
                for (int neighbor : neighbors(idx)) {
                    if (!done[neighbor] &&
                        m_weights[neighbor] > m_weights[idx]) {
                        wins = false;
                        break;
                    }
                }
                if (wins) {
                    localBatch.push_back(idx);
                }
            }

#pragma omp critical
            {
                for (int idx : localBatch) {
                    if (!done[idx]) {
                        done[idx] = 1;
                        color[idx] = (int)batches.size();
                        currentBatch.push_back(idx);
                        processedNodes++;
                    }
                }
            }
        }

        if (!currentBatch.empty()) {
            batches.push_back(currentBatch);
        }
    }

    return batches;
}

} // namespace Tissu