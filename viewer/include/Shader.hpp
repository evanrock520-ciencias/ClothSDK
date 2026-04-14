#pragma once

#include <string>
#include "Eigen/Dense"

namespace Tissu {

namespace Viewer {

class Shader {
public:
    Shader(std::string& vertPath, std::string& fragmentPath);

    void bind();
    void unbind();
    void reload();

    void setFloat(const std::string& name, int value) const;
    void setVec3(const std::string& name, const Eigen::Vector3f& value) const;
    void setMat4(const std::string& name, const Eigen::Matrix4f& value) const;

    inline bool isValid() const { return m_program != 0; }

private:
    unsigned int compile(const std::string& vertPath, const std::string& fragPath);
    std::string loadFile(const std::string& path) const;
    int getUniformLocation(const std::string& name) const;
    
    unsigned int m_program = 0;
    std::string m_vertPath;
    std::string m_fragmentPath;
};

}

}