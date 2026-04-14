#include "Shader.hpp"

namespace Tissu {

namespace Viewer {

Shader::Shader(std::string& vertPath, std::string& fragmentPath)
    : m_fragmentPath(fragmentPath), m_vertPath(vertPath) {}

}

}