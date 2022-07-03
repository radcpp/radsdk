#include "HelloWorld.h"

HelloWorld::HelloWorld(Ref<VulkanInstance> instance) :
    VulkanWindow(instance)
{
}

HelloWorld::~HelloWorld()
{
}

bool HelloWorld::Init()
{
    m_gpus = m_instance->EnumeratePhysicalDevices();
    if (m_gpus.empty())
    {
        LogPrint("HelloWorld", LogLevel::Info, "No Vulkan Device found!");
        return false;
    }

    Ref<VulkanPhysicalDevice> gpuPreferred;
    for (auto& gpu : m_gpus)
    {
        if (gpu->SupportsSurface(VulkanQueueFamilyUniversal, m_surface))
        {
            if (gpuPreferred == nullptr)
            {
                gpuPreferred = gpu;
            }
            if (gpu->GetProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            {
                gpuPreferred = gpu;
                break;
            }
        }
    }

    if (gpuPreferred)
    {
        m_device = gpuPreferred->CreateDevice();
        LogPrint("HelloWorld", LogLevel::Info, "Logical device created on '%s'", gpuPreferred->GetDeviceName());

        if (!VulkanWindow::Init(m_device))
        {
            return false;
        }

        m_renderer = MakeRefCounted<VulkanRenderer>(m_device, this);
        return true;
    }
    else
    {
        LogPrint("HelloWorld", LogLevel::Info, "No GPU supports the window surface!");
        return false;
    }
}

void HelloWorld::OnRender()
{
    VulkanCommandBuffer* cmdBuffer = BeginFrame();

    using Clock = std::chrono::high_resolution_clock;
    static Clock::time_point lastTime = Clock::now();
    Clock::time_point currTime = Clock::now();
    float deltaTime = std::chrono::duration<float>(currTime - lastTime).count();
    lastTime = currTime;

    if (m_renderer && m_cameraController)
    {
        int windowWidth = 0;
        int windowHeight = 0;
        GetSize(&windowWidth, &windowHeight);
        SDL_WarpMouseInWindow(m_window, windowWidth / 2, windowHeight / 2);
        m_cameraController->Update(deltaTime);
    }

    cmdBuffer->Begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
    std::array<VkClearValue, 2> clearValues;
    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 0.0f };
    clearValues[1].depthStencil = { 1.0, 0 };
    cmdBuffer->BeginRenderPass(m_defaultRenderPass.get(), m_defaultFramebuffers[m_swapchainImageIndex].get(), clearValues);
    m_renderer->Render(deltaTime);
    cmdBuffer->EndRenderPass();
    cmdBuffer->End();
    EndFrame();
}

void HelloWorld::OnResized(int width, int height)
{
    VulkanWindow::OnResized(width, height);

    m_viewports.resize(1);
    m_viewports[0].x = 0;
    m_viewports[0].y = 0;
    m_viewports[0].width = float(width);
    m_viewports[0].height = float(height);
    m_viewports[0].minDepth = 0.0f;
    m_viewports[0].maxDepth = 1.0f;
    m_scissors.resize(1);
    m_scissors[0].offset.x = 0;
    m_scissors[0].offset.y = 0;
    m_scissors[0].extent.width = width;
    m_scissors[0].extent.height = height;

    if (m_renderer)
    {
        m_renderer->Resize(width, height);
        m_renderer->SetViewports(m_viewports);
        m_renderer->SetScissors(m_scissors);
    }
}

void HelloWorld::OnClose()
{
    m_device->WaitIdle();
    m_renderer.reset();

    VulkanWindow::OnClose();
}

void HelloWorld::OnKeyDown(const SDL_KeyboardEvent& keyDown)
{
    if (keyDown.keysym.sym == SDLK_F1)
    {
        if (m_renderer)
        {
            if (m_cameraController)
            {
                m_cameraController.reset();
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
            else
            {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                m_cameraController = MakeRefCounted<CameraControllerFly>(m_renderer->GetScene()->m_camera.get());
                int windowWidth = 0;
                int windowHeight = 0;
                GetSize(&windowWidth, &windowHeight);
                int displayIndex = SDL_GetWindowDisplayIndex(m_window);
                SDL_DisplayMode displayMode = {};
                SDL_GetCurrentDisplayMode(displayIndex, &displayMode);
                m_cameraController->SetLookSpeed(180.0f / displayMode.w, 90.0f / displayMode.h);
            }
        }
    }

    if (m_cameraController)
    {
        m_cameraController->OnKeyDown(keyDown);
    }
}

void HelloWorld::OnKeyUp(const SDL_KeyboardEvent& keyUp)
{
    if (m_cameraController)
    {
        m_cameraController->OnKeyUp(keyUp);
    }
}

void HelloWorld::OnMouseMove(const SDL_MouseMotionEvent& mouseMotion)
{
    if (m_cameraController)
    {
        m_cameraController->OnMouseMove(mouseMotion);
    }
}
