#pragma once

#include <Eigen/Dense>

namespace ClothSDK {
namespace Viewer {

class Camera {
public:
    Camera(Eigen::Vector3f position = Eigen::Vector3f(0.0f, 5.0f, 15.0f), 
            Eigen::Vector3f target = Eigen::Vector3f(0.0f, 5.0f, 0.0f));

    Eigen::Matrix4f getViewMatrix() const;
    Eigen::Matrix4f getProjectionMatrix() const;

    void handleMouse(float xoffset, float yoffset);
    void handleZoom(float yoffset);
    void setAspectRatio(float ratio);

private:
    void updateCameraVectors();

    Eigen::Vector3f m_position; 
    Eigen::Vector3f m_target;
    Eigen::Vector3f m_up;

    float m_yaw;  
    float m_pitch; 
    float m_distance; 

    float m_fov;
    float m_aspectRatio;
    float m_near;
    float m_far;

};

}
}