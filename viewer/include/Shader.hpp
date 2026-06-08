#pragma once

#include "Eigen/Dense"
#include <string>

namespace Tissu {

namespace Viewer {

class Shader {
public:
  Shader(const std::string &vertPath = "",
         const std::string &fragmentPath = "");

  bool init();

  void bind() const;
  void unbind() const;
  void reload();

  void setFloat(const std::string &name, float value) const;
  void setVec3(const std::string &name, const Eigen::Vector3f &value) const;
  void setMat4(const std::string &name, const Eigen::Matrix4f &value) const;

  inline bool isValid() const { return m_program != 0; }
  inline unsigned int getProgram() const { return m_program; }

private:
  unsigned int compile(const std::string &vertPath,
                       const std::string &fragPath);
  std::string loadFile(const std::string &path) const;
  int getUniformLocation(const std::string &name) const;

  unsigned int m_program = 0;
  std::string m_vertPath;
  std::string m_fragmentPath;
};

} // namespace Viewer

} // namespace Tissu