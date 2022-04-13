#ifndef RADCPP_APPLICATION_H
#define RADCPP_APPLICATION_H
#pragma once

#include "radcpp/Common/Common.h"
#include "radcpp/Common/Containers.h"

#include "SDL2/SDL.h"

using WindowID = Uint32;

struct DisplayInfo
{
    std::string name;
    SDL_Rect bounds;
    SDL_Rect usableBounds;
    float ddpi;
    float hdpi;
    float vdpi;
    std::vector<SDL_DisplayMode> modes;
};

class Window;

class Application
{
public:
    Application(int argc, char** argv);
    ~Application();

    int Run();
    bool OnEvent(const SDL_Event& event);

    void SetExitEventLoop(bool exit = true) { m_exitEventLoop = exit; }
    void SetExitCode(int exitCode) { m_exitCode = exitCode; }

    const std::vector<std::string>& GetCommandLineArgs() const { return m_commandLineArgs; }
    const std::vector<DisplayInfo>& GetDisplays() const { return m_displayInfos; };
private:
    Window* GetWindowFromId(WindowID id);
    void UpdateDisplays();

    std::vector<std::string> m_commandLineArgs;
    std::vector<DisplayInfo> m_displayInfos;

    bool m_exitEventLoop = false;
    int m_exitCode = 0;

}; // class Application

class Window
{
    friend class Application;
public:
    Window();
    virtual ~Window();

    virtual bool Create(const char* title, int x, int y, int width, int height, Uint32 flags);
    virtual void Destroy();

    void SetShouldClose(bool shouldClose) { m_shouldClose = shouldClose; }
    bool ShouldClose() { return m_shouldClose; }

    SDL_Window* GetHandle() const { return m_window; }
    void GetPosition(int* x, int* y);
    void GetSize(int* w, int* h);
    void GetMaximumSize(int* w, int* h);
    void GetMinimumSize(int* w, int* h);
    void GetBorderSize(int* top, int* left, int* bottom, int* right);
    // Get the brightness (gamma multiplier) for a given window's display.
    // Returns the brightness for the display where 0.0 is completely dark and 1.0 is normal brightness.
    float GetBrightness();
    // Get the index of the display associated with a window.
    int GetDisplayIndex();
    int GetDisplayMode(SDL_DisplayMode* displayMode);
    int GetFlags();
    int GetGammaRamp(Uint16* r, Uint16* g, Uint16* b);
    bool IsGrabbed();
    float GetOpacity();
    // SDL_PIXELFORMAT
    Uint32 GetPixelFormat();
    const char* GetTitle();

    void Resize(int w, int h);
    void Hide();
    void Show();
    void Maximize();
    void Minimize();
    void Raise();
    void Restore();
    void SetBordered(bool bordered = true);
    int SetBrightness(float brightness);
    int SetDisplayMode(const SDL_DisplayMode& mode);
    int SetGammaRamp(const Uint16* r, const Uint16* g, const Uint16* b);
    void Grab(bool grabbed);
    void SetIcon(SDL_Surface* icon);
    void SetInputFocus();
    void SetMaximumSize(int w, int h);
    void SetMinimumSize(int w, int h);
    void SetTitle(std::string_view title);

protected:
    virtual bool OnEvent(const SDL_Event& event) { return false; }
    virtual void OnWindowEvent(const SDL_WindowEvent& windowEvent);
    virtual void OnShown() {}
    virtual void OnHidden() {}
    virtual void OnExposed() {}
    virtual void OnMoved(int x, int y) {}
    virtual void OnResized(int width, int height) {}
    virtual void OnMinimized() {}
    virtual void OnMaximized() {}
    virtual void OnRestored() {}
    virtual void OnEnter() {}
    virtual void OnLeave() {}
    virtual void OnFocusGained() {}
    virtual void OnFocusLost() {}
    virtual void OnClose();

    // Key
    virtual void OnKeyDown(const SDL_KeyboardEvent& keyDown) {}
    virtual void OnKeyUp(const SDL_KeyboardEvent& keyUp) {}
    virtual void OnTextEditing(const SDL_TextEditingEvent& textEditing) {}
    virtual void OnTextInput(const SDL_TextInputEvent& textInput) {}

    // Mouse
    virtual void OnMouseMove(const SDL_MouseMotionEvent& mouseMotion) {}
    virtual void OnMouseButtonDown(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseButtonUp(const SDL_MouseButtonEvent& mouseButton) {}
    virtual void OnMouseWheel(const SDL_MouseWheelEvent& mouseWheel ) {}

    virtual void OnRender() {}

    SDL_Window* m_window = nullptr;
    WindowID m_id = 0;
    bool m_shouldClose = false;

}; // class Window

extern Application* g_app;

#endif // RADCPP_APPLICATION_H