#ifndef WINDOW_TEST
#define WINDOW_TEST
#pragma once

#include "radcpp/Common/Application.h"
#include "radcpp/Common/Log.h"

class WindowTest : public Window
{
public:
    WindowTest()
    {
    }
    ~WindowTest()
    {
    }

    virtual void OnShown()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnShown");
    }
    virtual void OnHidden()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnHidden");
    }
    virtual void OnExposed()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnExposed");
    }
    virtual void OnMoved(int x, int y)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMoved");
    }
    virtual void OnResized(int width, int height)
    {
        Window::OnResized(width, height);
        LogPrint("WindowTest", LogLevel::Info, "OnResized");
    }
    virtual void OnMinimized()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMinimized");
    }
    virtual void OnMaximized()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMaximized");
    }
    virtual void OnRestored()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnRestored");
    }
    virtual void OnEnter()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnEnter");
    }
    virtual void OnLeave()
    {
        LogPrint("WindowTest", LogLevel::Info, "OnLeave");
    }

    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnKeyDown: %s", SDL_GetKeyName(keyDown.keysym.sym));
    }
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnKeyUp: %s", SDL_GetKeyName(keyUp.keysym.sym));
    }

    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMouseMove: Pos=(%4d, %4d)", mouseMotion.x, mouseMotion.y);
    }
    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMouseButtonDown: %s", GetMouseButtonName(mouseButton.button));
    }
    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMouseButtonUp: %s", GetMouseButtonName(mouseButton.button));
    }
    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel)
    {
        LogPrint("WindowTest", LogLevel::Info, "OnMouseWheel: %+d", mouseWheel.y);
    }

private:
    const char* GetMouseButtonName(Uint8 button)
    {
        switch (button)
        {
        case SDL_BUTTON_LEFT: return "SDL_BUTTON_LEFT";
        case SDL_BUTTON_MIDDLE: return "SDL_BUTTON_MIDDLE";
        case SDL_BUTTON_RIGHT: return "SDL_BUTTON_RIGHT";
        case SDL_BUTTON_X1: return "SDL_BUTTON_X1";
        case SDL_BUTTON_X2: return "SDL_BUTTON_X2";
        }
        return "SDL_BUTTON_UNKNOWN";
    }

}; // class WindowTest

#endif // WINDOW_TEST