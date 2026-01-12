#pragma once
#include <Eigen/Dense>
#include <vector>
#include <string>

namespace ClothSDK {
    class Solver;
    namespace Viewer {
        class Camera;

        class Renderer {
        public:
            Renderer();
            ~Renderer();

            bool init();
            void render(const ClothSDK::Solver& solver, const Camera& camera);
            void cleanup();

            inline void setIndices(const std::vector<unsigned int>& indices) { m_indices = indices; }

        private:
            unsigned int compileShaders(const std::string& vertexPath, const std::string& fragmentPath);
            std::string loadFile(const std::string& path);

            unsigned int m_shaderProgram = 0;
            unsigned int m_vao = 0;
            unsigned int m_vbo = 0;
            unsigned int m_ebo = 0;
            
            std::vector<float> m_vertexBuffer; 
            std::vector<unsigned int> m_indices;
        };
    }
}