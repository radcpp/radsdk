#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include "radcpp/Common/Application.h"
#include "radcpp/Common/Math.h"
#include "radcpp/VulkanEngine/VulkanCamera.h"

class CameraController : public RefCounted<CameraController>
{
public:
    CameraController(VulkanCamera* camera);
    ~CameraController();

    // Key
    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown) {}
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp) {}

    // Mouse
    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion) {}
    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel) {}

    virtual void Update(float deltaTime) {}

    virtual void SetMoveSpeed(float moveSpeed);
    virtual void SetLookSpeed(float lookSpeedHorizontal, float lookSpeedVertical);

protected:
    VulkanCamera* m_camera;

    float m_moveSpeed = 1.0f;

    float m_lookSpeedHorizontal = 1.0f;
    float m_lookSpeedVertical = 1.0f;

}; // class CameraController

class CameraControllerFly : public CameraController
{
public:
    CameraControllerFly(VulkanCamera* camera);
    ~CameraControllerFly();

    // Key
    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown);
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp);

    // Mouse
    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion);
    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel) {}

    virtual void Update(float deltaTime);

private:
    struct Input
    {
        bool moveForward;
        bool moveBackward;
        bool moveLeft;
        bool moveRight;
        bool moveUp;
        bool moveDown;
    } m_input = {};

    float m_currentHeading;
    float m_currentPitch;

}; // CameraControllerFly

#endif // CAMERA_CONTROLLER_H