#include "VulkanCamera.h"

VulkanCamera::VulkanCamera()
{
}

VulkanCamera::~VulkanCamera()
{
}

glm::mat4 VulkanCamera::GetViewMatrix()
{
    return glm::lookAt(m_position, m_position + m_direction, m_up);
}

glm::mat4 VulkanCamera::GetProjectionMatrix()
{
    return glm::perspective(m_yfov, m_aspectRatio, m_zNear, m_zFar);
}
