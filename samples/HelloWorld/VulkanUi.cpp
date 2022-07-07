#include "VulkanUi.h"

static void CheckResult(VkResult err)
{
    if (err == 0)
    {
        return;
    }
    LogPrint("Vulkan", LogLevel::Error, "ImGui error: VkResult=%s(%d).",
        string_VkResult(err), err);
    if (err < 0)
    {
        throw VulkanError(err);
    }
}

static PFN_vkVoidFunction GetVulkanProcAddr(const char* functionName, void* userData)
{
    VulkanInstance* instance = reinterpret_cast<VulkanUi*>(userData)->GetInstance();
    return vkGetInstanceProcAddr(instance->GetHandle(), functionName);
}

VulkanUi::VulkanUi(VulkanWindow* window) :
    m_window(window)
{
    IMGUI_CHECKVERSION();
    m_context = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    m_device = m_window->GetDevice();

    // TODO: what are the best sizes?
    VkDescriptorPoolSize descriptorPoolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1024 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1024 }
    };
    m_descriptorPool = m_device->CreateDescriptorPool(1024 * IM_ARRAYSIZE(descriptorPoolSizes), descriptorPoolSizes);

    ImGui_ImplSDL2_InitForVulkan(m_window->GetHandle());
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_window->GetInstance()->GetHandle();
    initInfo.PhysicalDevice = m_device->GetPhysicalDevice()->GetHandle();
    initInfo.Device = m_device->GetHandle();
    initInfo.QueueFamily = m_device->GetQueueFamilyIndex(VulkanQueueFamilyUniversal);
    initInfo.Queue = m_device->GetQueue(VulkanQueueFamilyUniversal)->GetHandle();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = m_descriptorPool->GetHandle();
    initInfo.Subpass = 0;
    initInfo.MinImageCount = m_window->GetSwapchain()->GetImageCount();
    initInfo.ImageCount = m_window->GetSwapchain()->GetImageCount();
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    initInfo.Allocator = nullptr;
    initInfo.CheckVkResultFn = CheckResult;

    ImGui_ImplVulkan_LoadFunctions(GetVulkanProcAddr, this);

    VulkanRenderPass* renderPass = m_window->GetDefaultRenderPass();
    ImGui_ImplVulkan_Init(&initInfo, renderPass->GetHandle());

    UploadFonts();
}

void VulkanUi::UploadFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 16.0f);
    Ref<VulkanCommandBuffer> commandBuffer = m_device->AllocateCommandBufferOneTimeUse();
    commandBuffer->Begin();
    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer->GetHandle());
    commandBuffer->End();
    m_device->GetQueue()->SubmitAndWaitForCompletion({ commandBuffer.get() });
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

VulkanUi::~VulkanUi()
{
    m_device->WaitIdle();
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
}

VulkanInstance* VulkanUi::GetInstance() const
{
    return m_window->GetInstance();
}

bool VulkanUi::OnEvent(const SDL_Event& event)
{
    bool result = ImGui_ImplSDL2_ProcessEvent(&event);
    return result;
}

void VulkanUi::OnResize(int width, int height)
{
    VulkanSwapchain* swapchain = m_window->GetSwapchain();
    ImGui_ImplVulkan_SetMinImageCount(swapchain->GetImageCount());
}

void VulkanUi::BeginFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void VulkanUi::EndFrame()
{
}

void VulkanUi::Render(VulkanCommandBuffer* commandBuffer)
{
    ImGui::Render();
    ImDrawData* drawData = ImGui::GetDrawData();
    const bool isMinimized = (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f);
    if (!isMinimized)
    {
        ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer->GetHandle());
    }
}
