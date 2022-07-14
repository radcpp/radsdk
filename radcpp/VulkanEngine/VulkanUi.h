#ifndef VULKAN_UI_H
#define VULKAN_UI_H

#include "VulkanCore.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_vulkan.h"

class VulkanUi : public RefCounted<VulkanUi>
{
public:
    VulkanUi(VulkanWindow* window);
    ~VulkanUi();

    VulkanInstance* GetInstance() const;

    bool OnEvent(const SDL_Event& event);
    void OnResize(int width, int height);

    void BeginFrame();
    void EndFrame();

    void Render(VulkanCommandBuffer* commandBuffer);

private:
    void UploadFonts();

    VulkanWindow* m_window = nullptr;
    VulkanDevice* m_device = nullptr;
    ImGuiContext* m_context = nullptr;

    Ref<VulkanDescriptorPool> m_descriptorPool;

};

#endif // VULKAN_UI_H