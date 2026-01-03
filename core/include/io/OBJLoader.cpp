#pragma once
#include <string>
#include <vector>
#include <Eigen/Dense>

namespace ClothSDK {

class OBJLoader {
public:
    static bool load(const std::string& filepath, std::vector<Eigen::Vector3d>& out_positions, std::vector<int>& out_indices);
};

}
