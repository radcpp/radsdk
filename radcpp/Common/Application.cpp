#include "radcpp/Common/Application.h"
#include "radcpp/Common/Exception.h"
#include "radcpp/Common/Log.h"

#if defined(_WIN32)
#include <Windows.h>
#endif

Application* g_app = nullptr;
std::map<WindowID, Window*> g_windowIds; // windows that registered for event loop

Application::Application(int argc, char** argv)
{
    assert(g_app == nullptr);
    g_app = this;

#if defined(_WIN32)
    ::SetConsoleOutputCP(65001);
#endif

    std::string commandLine;
    for (int i = 0; i < argc; i++)
    {
        m_commandLineArgs.push_back(argv[i]);
        commandLine += std::string(argv[i]) + " ";
    }
    commandLine.pop_back();
    LogPrint("Global", LogLevel::Info, "Command line: %s", commandLine.c_str());

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        LogPrint("Global", LogLevel::Error, "SDL_Init failed: %s", SDL_GetError());
        throw std::runtime_error(SDL_GetError());
    }

    SDL_version linkedVersion = {};
    SDL_GetVersion(&linkedVersion);
    LogPrint("Global", LogLevel::Info, "SDL initialized: Version: %d.%d.%d",
        linkedVersion.major, linkedVersion.minor, linkedVersion.patch);

    LogPrint("Global", LogLevel::Info, "Platform: %s", SDL_GetPlatform());
    char* basePath = SDL_GetBasePath();
    LogPrint("Global", LogLevel::Info, "BasePath: %s", basePath);
    SDL_free(basePath);

    UpdateDisplays();
}

Application::~Application()
{
    SDL_Quit();
    LogPrint("Global", LogLevel::Info, "SDL quited.");
}

int Application::Run()
{
    while (!m_exitEventLoop)
    {
        SDL_Event event = {};
        while (SDL_PollEvent(&event))
        {
            OnEvent(event);
        }

        for (auto& idHandlePair : g_windowIds)
        {
            if (idHandlePair.second)
            {
                idHandlePair.second->OnRender();
            }
        }

        if (g_windowIds.empty())
        {
            m_exitEventLoop = true;
        }
    }
    return m_exitCode;
}

Window* Application::GetWindowFromId(WindowID id)
{
    auto iter = g_windowIds.find(id);
    if (iter != g_windowIds.end())
    {
        return iter->second;
    }
    return nullptr;
}

bool Application::OnEvent(const SDL_Event& event)
{
    for (auto& idHandlePair : g_windowIds)
    {
        if (idHandlePair.second)
        {
            if (idHandlePair.second->OnEvent(event))
            {
                return true;
            }
        }
    }

    switch (event.type)
    {
    case SDL_QUIT:
        m_exitEventLoop = true;
        break;
    case SDL_WINDOWEVENT:
        if (Window* window = GetWindowFromId(event.window.windowID))
        {
            window->OnWindowEvent(event.window);
        }
        break;
    case SDL_KEYDOWN:
        if (Window* window = GetWindowFromId(event.key.windowID))
        {
            window->OnKeyDown(event.key);
        }
        break;
    case SDL_KEYUP:
        if (Window* window = GetWindowFromId(event.key.windowID))
        {
            window->OnKeyUp(event.key);
        }
        break;
    case SDL_TEXTEDITING:
        if (Window* window = GetWindowFromId(event.edit.windowID))
        {
            window->OnTextEditing(event.edit);
        }
        break;
    case SDL_TEXTINPUT:
        if (Window* window = GetWindowFromId(event.text.windowID))
        {
            window->OnTextInput(event.text);
        }
        break;
    case SDL_MOUSEMOTION:
        if (Window* window = GetWindowFromId(event.motion.windowID))
        {
            window->OnMouseMove(event.motion);
        }
        break;
    case SDL_MOUSEBUTTONDOWN:
        if (Window* window = GetWindowFromId(event.button.windowID))
        {
            window->OnMouseButtonDown(event.button);
        }
        break;
    case SDL_MOUSEBUTTONUP:
        if (Window* window = GetWindowFromId(event.button.windowID))
        {
            window->OnMouseButtonUp(event.button);
        }
        break;
    case SDL_MOUSEWHEEL:
        if (Window* window = GetWindowFromId(event.wheel.windowID))
        {
            window->OnMouseWheel(event.wheel);
        }
        break;

    case SDL_APP_TERMINATING:
        LogPrint("Global", LogLevel::Info, "SDL_APP_TERMINATING: The application is being terminated by the OS.");
        break;
    case SDL_APP_LOWMEMORY:
        LogPrint("Global", LogLevel::Info, "SDL_APP_LOWMEMORY: The application is low on memory, free memory if possible.");
        break;
    case SDL_APP_WILLENTERBACKGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_WILLENTERBACKGROUND: The application is about to enter the background.");
        break;
    case SDL_APP_DIDENTERBACKGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_DIDENTERBACKGROUND: The application did enter the background and may not get CPU for some time.");
        break;
    case SDL_APP_WILLENTERFOREGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_WILLENTERFOREGROUND: The application is about to enter the foreground.");
        break;
    case SDL_APP_DIDENTERFOREGROUND:
        LogPrint("Global", LogLevel::Info, "SDL_APP_DIDENTERFOREGROUND: The application is now interactive.");
        break;
    case SDL_LOCALECHANGED:
        LogPrint("Global", LogLevel::Info, "SDL_LOCALECHANGED: The user's locale preferences have changed.");
        break;
    case SDL_DISPLAYEVENT:
        LogPrint("Global", LogLevel::Info, "SDL_DISPLAYEVENT: Display state change.");
        UpdateDisplays();
        break;
    }

    return true;
}

void Application::UpdateDisplays()
{
    m_displayInfos.resize(SDL_GetNumVideoDisplays());
    for (int displayIndex = 0; displayIndex < static_cast<int>(m_displayInfos.size()); displayIndex++)
    {
        DisplayInfo& displayInfo = m_displayInfos[displayIndex];
        displayInfo.name = SDL_GetDisplayName(displayIndex);
        SDL_GetDisplayBounds(displayIndex, &displayInfo.bounds);
        SDL_GetDisplayUsableBounds(displayIndex, &displayInfo.usableBounds);
        SDL_GetDisplayDPI(displayIndex, &displayInfo.ddpi, &displayInfo.hdpi, &displayInfo.vdpi);
        displayInfo.modes.resize(SDL_GetNumDisplayModes(displayIndex));
        for (int modeIndex = 0; modeIndex < displayInfo.modes.size(); modeIndex++)
        {
            SDL_GetDisplayMode(displayIndex, modeIndex, &displayInfo.modes[modeIndex]);
        }
    }
}

Window::Window()
{
}

Window::~Window()
{
    Destroy();
}

bool Window::Create(const char* title, int x, int y, int width, int height, Uint32 flags)
{
    m_window = SDL_CreateWindow(title, x, y, width, height, flags);
    if (m_window)
    {
        m_id = SDL_GetWindowID(m_window);
        g_windowIds[m_id] = this;
        return true;
    }
    return false;
}

void Window::Destroy()
{
    if (m_window)
    {
        g_windowIds.erase(m_id);
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Window::GetPosition(int* x, int* y)
{
    SDL_GetWindowPosition(m_window, x, y);
}

void Window::GetSize(int* w, int* h)
{
    SDL_GetWindowSize(m_window, w, h);
}

void Window::GetMaximumSize(int* w, int* h)
{
    SDL_GetWindowMaximumSize(m_window, w, h);
}

void Window::GetMinimumSize(int* w, int* h)
{
    SDL_GetWindowMinimumSize(m_window, w, h);
}

void Window::GetBorderSize(int* top, int* left, int* bottom, int* right)
{
    SDL_GetWindowBordersSize(m_window, top, left, bottom, right);
}

float Window::GetBrightness()
{
    return SDL_GetWindowBrightness(m_window);
}

int Window::GetDisplayIndex()
{
    return SDL_GetWindowDisplayIndex(m_window);
}

int Window::GetDisplayMode(SDL_DisplayMode* displayMode)
{
    return SDL_GetWindowDisplayMode(m_window, displayMode);
}

int Window::GetFlags()
{
    return SDL_GetWindowFlags(m_window);
}

int Window::GetGammaRamp(Uint16* r, Uint16* g, Uint16* b)
{
    return SDL_GetWindowGammaRamp(m_window, r, g, b);
}

bool Window::IsGrabbed()
{
    return (SDL_GetWindowGrab(m_window) == SDL_TRUE);
}

float Window::GetOpacity()
{
    float opacity = 1.0f;
    SDL_GetWindowOpacity(m_window, &opacity);
    return opacity;
}

Uint32 Window::GetPixelFormat()
{
    return SDL_GetWindowPixelFormat(m_window);
}

const char* Window::GetTitle()
{
    return SDL_GetWindowTitle(m_window);
}


void Window::Resize(int w, int h)
{
    SDL_SetWindowSize(m_window, w, h);
}

void Window::Hide()
{
    SDL_HideWindow(m_window);
}

void Window::Show()
{
    SDL_ShowWindow(m_window);
}

void Window::Maximize()
{
    SDL_MaximizeWindow(m_window);
}

void Window::Minimize()
{
    SDL_MinimizeWindow(m_window);
}

void Window::Raise()
{
    SDL_RaiseWindow(m_window);
}

void Window::Restore()
{
    SDL_RestoreWindow(m_window);
}

void Window::SetBordered(bool bordered)
{
    SDL_SetWindowBordered(m_window, bordered ? SDL_TRUE : SDL_FALSE);
}

int Window::SetBrightness(float brightness)
{
    return SDL_SetWindowBrightness(m_window, brightness);
}

int Window::SetDisplayMode(const SDL_DisplayMode& mode)
{
    return SDL_SetWindowDisplayMode(m_window, &mode);
}

int Window::SetGammaRamp(const Uint16* r, const Uint16* g, const Uint16* b)
{
    return SDL_SetWindowGammaRamp(m_window, r, g, b);
}

void Window::Grab(bool grabbed)
{
    SDL_SetWindowGrab(m_window, grabbed ? SDL_TRUE : SDL_FALSE);
}

void Window::SetIcon(SDL_Surface* icon)
{
    SDL_SetWindowIcon(m_window, icon);
}

void Window::SetInputFocus()
{
    SDL_SetWindowInputFocus(m_window);
}

void Window::SetMaximumSize(int w, int h)
{
    SDL_SetWindowMaximumSize(m_window, w, h);
}

void Window::SetMinimumSize(int w, int h)
{
    SDL_SetWindowMinimumSize(m_window, w, h);
}

void Window::SetTitle(std::string_view title)
{
    SDL_SetWindowTitle(m_window, title.data());
}

void Window::OnWindowEvent(const SDL_WindowEvent& windowEvent)
{
    switch (windowEvent.event)
    {
    case SDL_WINDOWEVENT_SHOWN:
        //window has been shown
        OnShown();
        break;
    case SDL_WINDOWEVENT_HIDDEN:
        // window has been hidden
        OnHidden();
        break;
    case SDL_WINDOWEVENT_EXPOSED:
        // window has been exposedand should be redrawn
        OnExposed();
        break;
    case SDL_WINDOWEVENT_MOVED:
        // window has been moved to data1, data2
        OnMoved(windowEvent.data1, windowEvent.data2);
        break;
    case SDL_WINDOWEVENT_RESIZED:
        // window has been resized to data1xdata2; this event is always preceded by SDL_WINDOWEVENT_SIZE_CHANGED
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        // window size has changed, either as a result of an API call or through the system or user changing the window size; this event is followed by SDL_WINDOWEVENT_RESIZED if the size was changed by an external event, i.e.the user or the window manager
        OnResized(windowEvent.data1, windowEvent.data2);
        break;
    case SDL_WINDOWEVENT_MINIMIZED:
        // window has been minimized
        OnMinimized();
        break;
    case SDL_WINDOWEVENT_MAXIMIZED:
        // window has been maximized
        OnMaximized();
        break;
    case SDL_WINDOWEVENT_RESTORED:
        // window has been restored to normal size and position
        OnRestored();
        break;
    case SDL_WINDOWEVENT_ENTER:
        // window has gained mouse focus
        OnEnter();
        break;
    case SDL_WINDOWEVENT_LEAVE:
        // window has lost mouse focus
        OnLeave();
        break;
    case SDL_WINDOWEVENT_FOCUS_GAINED:
        // window has gained keyboard focus
        OnFocusGained();
        break;
    case SDL_WINDOWEVENT_FOCUS_LOST:
        // window has lost keyboard focus
        OnFocusLost();
        break;
    case SDL_WINDOWEVENT_CLOSE:
        // the window manager requests that the window be closed
        OnClose();
        break;
    }
}

void Window::OnClose()
{
    m_shouldClose = true;
    Destroy();
}

std::string GetEnv(std::string_view varName)
{
    std::string var;
#ifdef _WIN32
    std::wstring varNameWide = StrU8ToWide(varName);
    wchar_t* pBuffer = nullptr;
    size_t bufferSize = 0;
    errno_t err = _wdupenv_s(&pBuffer, &bufferSize, varNameWide.data());
    if (err == 0)
    {
        if (pBuffer)
        {
            var = StrWideToU8(pBuffer);
            free(pBuffer);
        }
    }
#else
    var = std::getenv(varName.data());
#endif
    return var;
}
