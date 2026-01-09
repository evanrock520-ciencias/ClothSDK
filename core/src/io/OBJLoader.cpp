#include <vector>
#define TINYOBJLOADER_IMPLEMENTATION
#include "io/OBJLoader.hpp"
#include <tiny_obj_loader.h>
#include <iostream>

namespace ClothSDK {

bool OBJLoader::load(const std::string& path, std::vector<Eigen::Vector3d>& outPos, std::vector<int>& outIndices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());

    if (!warn.empty()) std::cout << "OBJ Warning: " << warn << std::endl;
    if (!err.empty()) std::cerr << "OBJ Error: " << err << std::endl;
    if (!ret) return false;

    size_t numVertices = attrib.vertices.size() / 3;
    outPos.reserve(numVertices);

    for (size_t i = 0; i < numVertices; ++i) {
        double vx = static_cast<double>(attrib.vertices[3 * i + 0]);
        double vy = static_cast<double>(attrib.vertices[3 * i + 1]);
        double vz = static_cast<double>(attrib.vertices[3 * i + 2]);
        
        outPos.emplace_back(vx, vy, vz);
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            outIndices.push_back(index.vertex_index);
        }
    }

    return true;
}

}
