/*
 * Copyright 2026 Evan M.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "engine/Cloth.hpp"
#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

namespace ClothSDK {

class Solver;
class ClothMesh;
class World;

namespace Viewer {

class Renderer;
class Camera;

class Application {
public:
    Application();
    ~Application();
    
    bool init(int width, int height, const std::string& title, const std::string& shaderPath);
    void run();
    void shutdown();
    void syncVisualTopology();

    inline void setWorld(std::shared_ptr<World> world) { m_world = world; }
    inline void setSolver(std::shared_ptr<Solver> solver) { m_solver = solver; }
    inline void setMesh(std::shared_ptr<ClothMesh> mesh) { m_mesh = mesh; }
    inline void setCloth(std::shared_ptr<Cloth> cloth) { m_cloth = cloth; }
    inline Renderer& getRenderer() { return *m_renderer; }

private:
    void processInput();
    void update();
    void render();
    void drawUI();
    void resetSimulation();

    GLFWwindow* m_window;
    std::shared_ptr<World> m_world;
    std::shared_ptr<Solver> m_solver;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Camera> m_camera;
    std::shared_ptr<ClothMesh> m_mesh; 
    std::shared_ptr<Cloth> m_cloth;
    double m_deltaTime;
    double m_lastFrame;

    double m_lastX = 0.0;
    double m_lastY = 0.0;
    bool m_firstMouse = true;

    bool m_isPaused;
    bool m_isGridScene;
    int m_initRows, m_initCols;
    double m_initSpacing;
    char m_configPathBuffer[256] = "data/configs/silk.json";

    std::vector<Eigen::Vector3d> m_originalPositions;
    std::vector<int> m_originalIndices;
};

}
}