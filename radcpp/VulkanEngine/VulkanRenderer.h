#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H
#pragma once

#include "VulkanCore.h"
#include "VulkanScene.h"

class VulkanRenderer : public RefCounted<VulkanRenderer>
{
public:
    VulkanRenderer(Ref<VulkanDevice> device, VulkanWindow* window);
    ~VulkanRenderer();

    VulkanScene* GetScene() const { return m_scene.get(); }

    bool Import3DModel(const Path& filePath);
    void Reset();

    void Resize(uint32_t width, uint32_t height);
    void SetViewports(ArrayRef<VkViewport> viewports) { m_viewports = viewports; }
    void SetScissors(ArrayRef<VkRect2D> scissors) { m_scissors = scissors; }

    void Render(float deltaTime);

private:
    void CreateSamplers();
    void CreateSolidWireframePipeline(VulkanMesh* mesh);
    std::vector<ShaderMacro> GetShaderMacros(VulkanMesh* mesh);
    void SetVertexInputState(VulkanGraphicsPipelineCreateInfo& pipelineInfo, VulkanMesh* mesh);

    void RenderNodes(VulkanSceneNode* node, glm::mat4 transform);

    Ref<VulkanDevice> m_device;
    Ref<VulkanScene> m_scene;
    VulkanWindow* m_window;

    struct FrameUniforms
    {
        glm::mat4 viewProjectionMatrix;
    };
    struct MeshUniforms
    {
        glm::mat4 modelToWorld;
    };
    std::vector<Ref<VulkanBuffer>> m_uniformBuffers;
    std::vector<uint8_t*> m_uniformData;
    VkDeviceSize m_uniformOffset = 0;
    uint32_t WriteUniforms(void* data, VkDeviceSize dataSize);

    Ref<VulkanDescriptorPool> m_samplerDescriptorPool;
    Ref<VulkanDescriptorSetLayout> m_samplerSetLayout;
    Ref<VulkanDescriptorSet> m_samplerSet;
    Ref<VulkanSampler> m_sampler;

    Ref<VulkanPipelineLayout> m_pipelineLayout;
    Ref<VulkanDescriptorPool> m_descriptorPool;
    Ref<VulkanDescriptorSetLayout> m_frameDescriptorSetLayout;
    std::vector<Ref<VulkanDescriptorSet>> m_frameDescriptorSets;
    Ref<VulkanDescriptorSetLayout> m_meshDescriptorSetLayout;
    std::string m_shaderSourceDir;
    std::map<std::string, Ref<VulkanGraphicsPipeline>> m_solidWireframePipelines;

    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;

}; // class VulkanRenderer

#endif // VULKAN_RENDERER_H