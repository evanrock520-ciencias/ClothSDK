#pragma once

#include <string>
namespace ClothSDK {

class Solver;
class Cloth;

class OBJExporter {
public:
    OBJExporter();
    ~OBJExporter();
    
    static void exportOBJ(const std::string &filename, const Cloth& cloth, const Solver &solver);
};

}