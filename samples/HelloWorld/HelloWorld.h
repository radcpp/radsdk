#ifndef HELLOWORLD_H
#define HELLOWORLD_H
#pragma once

#include "radcpp/VulkanEngine/VulkanRenderer.h"
#include "CameraController.h"

class HelloWorld : public VulkanWindow
{
public:
    HelloWorld(Ref<VulkanInstance> instance);
    ~HelloWorld();

    bool Init();

    VulkanRenderer* GetRenderer() const { return m_renderer.get(); }

protected:
    virtual void OnShown() {}
    virtual void OnHidden() {}
    virtual void OnExposed() {}
    virtual void OnMoved(int x, int y) {}
    virtual void OnResized(int width, int height);
    virtual void OnMinimized() {}
    virtual void OnMaximized() {}
    virtual void OnRestored() {}
    virtual void OnEnter() {}
    virtual void OnLeave() {}
    virtual void OnFocusGained() {}
    virtual void OnFocusLost() {}
    virtual void OnClose();

    // Key
    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown);
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp);
    virtual void OnTextEditing(const SDL_TextEditingEvent& textEditing) {}
    virtual void OnTextInput(const SDL_TextInputEvent& textInput) {}

    // Mouse
    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion);
    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel) {}

    virtual void OnRender() override;

private:
    std::vector<Ref<VulkanPhysicalDevice>> m_gpus;
    Ref<VulkanRenderer> m_renderer;
    Ref<CameraController> m_cameraController;

    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;

}; // class HelloWorld

#endif // HELLOWORLD_H