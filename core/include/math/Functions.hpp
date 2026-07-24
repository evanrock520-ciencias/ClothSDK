#pragma once

#include "Eigen/Dense"

namespace Tissu {
inline Eigen::Vector3d lerp(const Eigen::Vector3d& a, const Eigen::Vector3d& b,
                            const double& t) {
    return (1 - t) * a + t * b;
}
} // namespace Tissu