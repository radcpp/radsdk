#include "CameraController.h"
#include "radcpp/Common/Math.h"
#include <algorithm>

CameraController::CameraController(VulkanCamera* camera)
{
    assert(camera != nullptr);
    m_camera = camera;
}

CameraController::~CameraController()
{
}

void CameraController::SetMoveSpeed(float moveSpeed)
{
    m_moveSpeed = moveSpeed;
}

void CameraController::SetLookSpeed(float lookSpeedHorizontal, float lookSpeedVertical)
{
    m_lookSpeedHorizontal = lookSpeedHorizontal;
    m_lookSpeedVertical = lookSpeedVertical;
}

CameraControllerFly::CameraControllerFly(VulkanCamera* camera) :
    CameraController(camera)
{
    glm::vec3 direction = glm::normalize(camera->m_direction);
    m_currentHeading = atan2(direction.x, direction.z);
    m_currentPitch = asin(direction.y);
}

CameraControllerFly::~CameraControllerFly()
{
}

void CameraControllerFly::OnKeyDown(const SDL_KeyboardEvent& keyDown)
{
    switch (keyDown.keysym.sym)
    {
    case SDL_KeyCode::SDLK_w:
        m_input.moveForward = true;
        break;
    case SDL_KeyCode::SDLK_s:
        m_input.moveBackward = true;
        break;
    case SDL_KeyCode::SDLK_a:
        m_input.moveLeft = true;
        break;
    case SDL_KeyCode::SDLK_d:
        m_input.moveRight = true;
        break;
    case SDL_KeyCode::SDLK_q:
        m_input.moveDown = true;
        break;
    case SDL_KeyCode::SDLK_e:
        m_input.moveUp = true;
        break;
    }
}

void CameraControllerFly::OnKeyUp(const SDL_KeyboardEvent& keyUp)
{
    switch (keyUp.keysym.sym)
    {
    case SDL_KeyCode::SDLK_w:
        m_input.moveForward = false;
        break;
    case SDL_KeyCode::SDLK_s:
        m_input.moveBackward = false;
        break;
    case SDL_KeyCode::SDLK_a:
        m_input.moveLeft = false;
        break;
    case SDL_KeyCode::SDLK_d:
        m_input.moveRight = false;
        break;
    case SDL_KeyCode::SDLK_q:
        m_input.moveDown = false;
        break;
    case SDL_KeyCode::SDLK_e:
        m_input.moveUp = false;
        break;
    }
}

void CameraControllerFly::OnMouseMove(const SDL_MouseMotionEvent& mouseMotion)
{
    m_currentHeading -= glm::radians(float(mouseMotion.xrel)) * m_lookSpeedHorizontal;
    m_currentPitch -= glm::radians(float(mouseMotion.yrel)) * m_lookSpeedVertical;
}

void CameraControllerFly::Update(float deltaTime)
{
    if (m_currentHeading > float(+MATH_PI))
    {
        m_currentHeading -= float(2.0 * MATH_PI);
    }
    else if (m_currentHeading < float(-MATH_PI))
    {
        m_currentHeading += float(2.0 * MATH_PI);
    }
    m_currentPitch = std::clamp(m_currentPitch, float(-0.9 * MATH_PI_2), float(+0.9 * MATH_PI_2));

    glm::vec3 direction;
    direction.x = cos(m_currentPitch) * sin(m_currentHeading);
    direction.y = sin(m_currentPitch);
    direction.z = cos(m_currentPitch) * cos(m_currentHeading);
    direction = glm::normalize(direction);

    glm::vec3 up = glm::normalize(m_camera->m_up);
    glm::vec3 right = glm::normalize(glm::cross(direction, up));

    if (m_input.moveForward)
    {
        m_camera->m_position += direction * m_moveSpeed * deltaTime;
    }
    if (m_input.moveBackward)
    {
        m_camera->m_position -= direction * m_moveSpeed * deltaTime;
    }
    if (m_input.moveLeft)
    {
        m_camera->m_position -= right * m_moveSpeed * deltaTime;
    }
    if (m_input.moveRight)
    {
        m_camera->m_position += right * m_moveSpeed * deltaTime;
    }
    if (m_input.moveUp)
    {
        m_camera->m_position += up * m_moveSpeed * deltaTime;
    }
    if (m_input.moveDown)
    {
        m_camera->m_position -= up * m_moveSpeed * deltaTime;
    }

    m_camera->m_direction = direction;
    m_camera->m_up = up;
}
