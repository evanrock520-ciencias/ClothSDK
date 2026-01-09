#pragma once
#include <string>
#include <vector>
#include <Eigen/Dense>

namespace ClothSDK {

class OBJLoader {
public:
    static bool load(const std::string& path, std::vector<Eigen::Vector3d>& outPos, std::vector<int>& outIndices);
};

}
