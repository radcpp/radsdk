#include "HelloWorld.h"
#include "radcpp/Common/NativeFileDialog.h"
#include "radcpp/Common/Process.h"

#include <future>
#include <thread>

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
        m_ui = MakeRefCounted<VulkanUi>(this);
        return true;
    }
    else
    {
        LogPrint("HelloWorld", LogLevel::Info, "No GPU supports the window surface!");
        return false;
    }
}

bool HelloWorld::OnEvent(const SDL_Event& event)
{
    m_ui->OnEvent(event);

    // Continue to process the event
    return false;
}

void HelloWorld::OnRender()
{
    VulkanCommandBuffer* cmdBuffer = BeginFrame();

    using Clock = std::chrono::high_resolution_clock;
    static Clock::time_point lastTime = Clock::now();
    Clock::time_point currTime = Clock::now();
    float deltaTime = std::chrono::duration<float>(currTime - lastTime).count();
    lastTime = currTime;

    m_ui->BeginFrame();
    if (m_showMainMenu)
    {
        ShowMainMenuBar();
    }
    if (m_showEnvironmentVariableTable)
    {
        ShowEnvironmentVariableTable();
    }

    if (m_cameraController)
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
    m_ui->Render(cmdBuffer);
    cmdBuffer->EndRenderPass();
    cmdBuffer->End();

    m_ui->EndFrame();
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

    if (m_ui)
    {
        m_ui->OnResize(width, height);
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

    if (keyDown.keysym.mod & KMOD_CTRL)
    {
        if (keyDown.keysym.sym == SDLK_o)
        {
            Import3DFile();
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

bool HelloWorld::Import3DFile()
{
    Path filePath;
    NativeFileDialog fileDialog;
    if (fileDialog.Init())
    {
        if (fileDialog.OpenFileDialog(filePath, { {"3D Model", "gltf"} }) == NativeFileDialog::ResultOkay)
        {
            LogPrint("HelloWorld", LogLevel::Info, "FileDialog: %s", (const char*)filePath.u8string().c_str());
            m_renderer->Import3DModel(filePath);
        }
    }
    fileDialog.Quit();
    return true;
}

void HelloWorld::ShowMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}  // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
            if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("System"))
        {
            if (ImGui::MenuItem("Environment Variables..."))
            {
                m_showEnvironmentVariableTable = true;
                m_environmentVariableQueryReady = false;
                std::thread t(&HelloWorld::QueryEnvironmentVariables, this);
                t.detach();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void HelloWorld::ShowMenuFile()
{
    if (ImGui::BeginMenu("New"))
    {
        if (ImGui::MenuItem("Project..."))
        {
            LogPrint("HelloWorld", LogLevel::Info, "MainMenu->New->Project...");
        }
        if (ImGui::MenuItem("File..."))
        {
            LogPrint("HelloWorld", LogLevel::Info, "MainMenu->New->File...");
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Open", "Ctrl+O"))
    {
        Import3DFile();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Save", "Ctrl+S"))
    {
    }
    if (ImGui::MenuItem("Save As...", "Ctrl+S"))
    {
    }
    if (ImGui::MenuItem("Save All", "Ctrl+Shift+S"))
    {
    }
    ImGui::Separator();

    if (ImGui::MenuItem("Quit", "Alt+F4"))
    {
    }
}

void HelloWorld::ShowEnvironmentVariableTable()
{
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    ImGui::Begin("Environment Variables", &m_showEnvironmentVariableTable);
    if (m_environmentVariableQueryReady)
    {
        if (ImGui::BeginTable("Environment Variable Table", 2,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders))
        {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.4f);
            ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();
            for (int row = 0; row < static_cast<int>(m_environmentVariables.size()); row++)
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(m_environmentVariables[row].name.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(m_environmentVariables[row].value.c_str());
            }
            ImGui::EndTable();
        }
    }
    else
    {
        ImGui::Text("Querying...");
    }
    ImGui::End();

    if (!m_showEnvironmentVariableTable)
    {
        m_environmentVariables.clear();
        m_environmentVariableQueryReady = false;
    }
}

void HelloWorld::QueryEnvironmentVariables()
{
    auto list = QProcess::systemEnvironment();
    m_environmentVariables.clear();
    for (const QString& qstr : list)
    {
        std::string str = qstr.toStdString();
        EnvironmentVariable var;
        var.name = str.substr(0, str.find_first_of('='));
        var.value = str.substr(var.name.size() + 1);

        if (var.name == "Path")
        {
            auto paths = StrSplit(var.value, ";");
            var.value.clear();
            for (const auto& path : paths)
            {
                var.value += path;
                var.value += '\n';
            }
            var.value.pop_back();
        }

        m_environmentVariables.push_back(var);
    }
    m_environmentVariableQueryReady = true;
}
