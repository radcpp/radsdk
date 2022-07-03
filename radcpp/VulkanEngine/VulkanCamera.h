#ifndef VULKAN_CAMERA_H
#define VULKAN_CAMERA_H
#pragma once

#include "radcpp/Common/Math.h"
#include "radcpp/Common/Memory.h"

class VulkanCamera : public RefCounted<VulkanCamera>
{
public:
    enum class Type
    {
        Perspective,
        Orthographic,
    };

    VulkanCamera();
    ~VulkanCamera();

    // assume all directions are normalized
    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

    std::string m_name;
    Type m_type = Type::Perspective;
    glm::vec3 m_position = { 0, 0, 0 };
    glm::vec3 m_direction = { 0, 0, 1 };
    glm::vec3 m_up = { 0, 1, 0 };
    float m_aspectRatio = 4.0f / 3.0f;
    // float xfov = 2.0f * atan(tan(yfov / 2.0f) * m_aspectRatio);
    // float yfov = 2.0f * atan(tan(xfov / 2.0f) / m_aspectRatio);
    float m_yfov = glm::radians(30.0f);
    float m_zNear = 0.001f;
    float m_zFar = 1000.0f;
    float m_xmag = 0.0f;   // ortho scale
    float m_ymag = 0.0f;   // ortho scale

    glm::vec2 m_sensorSize;
    float m_focalLength = 0.0f;
    // float xfov = 2.0f * atan(m_sensorSize.x / 2.0f / m_focalLength);
    // float yfov = 2.0f * atan(m_sensorSize.y / 2.0f / m_focalLength);

    // depth of field
    float m_focalDistance = 0.0f;

    struct Aperture
    {
        float fstop;        // F-Stop ratio that defines the amount of blurring. Lower values give a strong depth of field effect.
        uint32_t blades;    // Total number of polygonal blades used to alter the shape of the blurred objects.
        float rotation;     // Rotate the polygonal blades along the facing axis.
        float ratio;        // Change the amount of distortion to simulate the anamorphic bokeh effect.
    } m_aperture = {};

}; // class VulkanCamera

#endif // VULKAN_CAMERA_H