#include "Camera.hpp"
#include <cmath>

namespace ClothSDK {
namespace Viewer {

static constexpr float PI = 3.1415926535f;
static constexpr float RAD_TO_DEG = 180.0f / PI;

Camera::Camera(Eigen::Vector3f position, Eigen::Vector3f target)
    : m_position(position), 
    m_target(target), 
    m_up(0.0f, 1.0f, 0.0f),
    m_fov(45.0f), 
    m_aspectRatio(1.77f), 
    m_near(0.1f), 
    m_far(1000.0f) 
{
    Eigen::Vector3f dir = m_position - m_target;

    m_distance = dir.norm();

    if (m_distance < 0.001f) {
        m_distance = 1.0f;
        dir = Eigen::Vector3f(0.0f, 0.0f, 1.0f);
    }

    dir.normalize();

    m_pitch = std::asin(dir.y()) * RAD_TO_DEG;
    
    m_yaw = std::atan2(dir.x(), dir.z()) * RAD_TO_DEG;

    updateCameraVectors();
}

Eigen::Matrix4f Camera::getProjectionMatrix() const {
    Eigen::Matrix4f projection = Eigen::Matrix4f::Zero();

    float tanHalfFov = std::tan((m_fov * RAD_TO_DEG) / 2.0f);

    projection(0, 0) = 1.0f / (m_aspectRatio * tanHalfFov);
    projection(1, 1) = 1.0f / tanHalfFov;
    projection(2, 2) = -(m_far + m_near) / (m_far - m_near);
    projection(2, 3) = -(2.0f * m_far * m_near) / (m_far - m_near);
    projection(3, 2) = -1.0f;

    return projection;
}

Eigen::Matrix4f Camera::getViewMatrix() const {
    Eigen::Vector3f w = (m_position - m_target).normalized();
    Eigen::Vector3f u = (m_up.cross(w)).normalized();
    Eigen::Vector3f v = w.cross(u);

    Eigen::Matrix4f view;
    view << u.x(), u.y(), u.z(), -(u.dot(m_position)),
            v.x(), v.y(), v.z(), -(v.dot(m_position)),
            w.x(), w.y(), w.z(), -(w.dot(m_position)),
            0    , 0    , 0    ,   1;

    return view;
}

void Camera::handleMouse(float xoffset, float yoffset) {
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    m_yaw   += xoffset;
    m_pitch += yoffset;

    if (m_pitch > 89.0f)  m_pitch = 89.0f;
    if (m_pitch < -89.0f) m_pitch = -89.0f;

    updateCameraVectors();
}

void Camera::handleZoom(float yoffset) {
    float zoomSpeed = 0.5f;
    m_distance -= yoffset * zoomSpeed;

    if (m_distance < 0.1f) m_distance = 0.1f;
    if (m_distance > 100.0f) m_distance = 100.0f;

    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    float yaw_rad = m_yaw * RAD_TO_DEG;
    float pitch_rad = m_pitch * RAD_TO_DEG;

    float cos_pitch = std::cos(pitch_rad);
    float y = m_distance * std::sin(pitch_rad);
    float x = m_distance * cos_pitch * std::sin(yaw_rad);
    float z = m_distance * cos_pitch * std::cos(yaw_rad);

    Eigen::Vector3f dis(x, y, z);
    m_position = m_target + dis;
}

}

}