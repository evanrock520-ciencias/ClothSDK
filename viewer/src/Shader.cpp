#include "Shader.hpp"
#include "utils/Logger.hpp"
#include <glad/gl.h>
#include <fstream>
#include <sstream>
#include <string>

namespace Tissu {

namespace Viewer {

Shader::Shader(const std::string& vertPath, const std::string& fragmentPath)
    : m_fragmentPath(fragmentPath), m_vertPath(vertPath) {}

bool Shader::init() {
    m_program = compile(m_vertPath, m_fragmentPath);
    return m_program != 0;
}

std::string Shader::loadFile(const std::string& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        Logger::error("Could not open shader file: " + path);
        return "";
    }

    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::compile(const std::string& vertPath, const std::string& fragPath) {
    std::string vCode = loadFile(vertPath);
    std::string fCode = loadFile(fragPath);
    if (vCode.empty() || fCode.empty()) return 0;

    const char* vShaderCode = vCode.c_str();
    const char* fShaderCode = fCode.c_str();

    int success;
    char infoLog[512];

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        Logger::error("Vertex Shader Compilation Failed: " + std::string(infoLog));
    }

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        Logger::error("Fragment Shader Compilation Failed: " + std::string(infoLog));
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        Logger::error("Shader Linking Failed: " + std::string(infoLog));
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return m_program;
}

void Shader::bind() const {
    glUseProgram(m_program);
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::reload() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
        m_program = 0;
    }
    m_program = compile(m_vertPath, m_fragmentPath);
}

int Shader::getUniformLocation(const std::string& name) const {
    return glGetUniformLocation(m_program, name.c_str());
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec3(const std::string& name, const Eigen::Vector3f& value) const {
    glUniform3fv(getUniformLocation(name), 1, value.data());
}

void Shader::setMat4(const std::string& name, const Eigen::Matrix4f& value) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, value.data());
}

}

}