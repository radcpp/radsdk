#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer(Ref<VulkanDevice> device, VulkanWindow* window) :
    m_device(std::move(device)),
    m_window(window)
{
    m_shaderSourceDir = GetEnv("RAD_SHADER_SOURCE_DIR");
    if (m_shaderSourceDir.empty() || !FileSystem::Exists((char8_t*)m_shaderSourceDir.data()))
    {
        m_shaderSourceDir = "Shaders/";
    }
#ifdef _WIN32
    if (!m_shaderSourceDir.ends_with('/') || !m_shaderSourceDir.ends_with('\\'))
#else
    if (!m_shaderSourceDir.ends_with('/'))
#endif
    {
        m_shaderSourceDir.push_back('/');
    }

    VulkanSwapchain* swapchain = m_window->GetSwapchain();

    m_uniformBuffers.resize(swapchain->GetImageCount());
    m_uniformData.resize(swapchain->GetImageCount());
    for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
    {
        m_uniformBuffers[i] = m_device->CreateUniformBuffer(128 * 1024 * 1024, true);
        m_uniformData[i] = (uint8_t*)m_uniformBuffers[i]->GetPersistentMappedAddr();
    }

    uint32_t maxSets = 4096;
    m_descriptorPool = m_device->CreateDescriptorPool(maxSets,
        std::array{ // type, descriptorCount
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    2 * swapchain->GetImageCount() },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,            1024 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,             4096 },
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,             4096 },
        });

    m_frameDescriptorSetLayout = m_device->CreateDescriptorSetLayout(
        std::array{ // layout bindings
        // binding, type, count, stageFlags, pImmutableSamplers
        VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        VkDescriptorSetLayoutBinding{ 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_ALL, nullptr},
        }
    );

    m_frameDescriptorSets.resize(swapchain->GetImageCount());
    for (uint32_t i = 0; i < swapchain->GetImageCount(); i++)
    {
        m_frameDescriptorSets[i] = m_descriptorPool->Allocate(m_frameDescriptorSetLayout.get());
        VkWriteDescriptorSet descWrites[2] = {};
        descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrites[0].pNext = nullptr;
        descWrites[0].dstSet = m_frameDescriptorSets[i]->GetHandle();
        descWrites[0].dstBinding = 0;
        descWrites[0].dstArrayElement = 0;
        descWrites[0].descriptorCount = 1;
        descWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descWrites[0].pImageInfo = nullptr;
        VkDescriptorBufferInfo frameUniformBufferInfo =
        {
            VkDescriptorBufferInfo{ m_uniformBuffers[i]->GetHandle(), 0, sizeof(FrameUniforms) }
        };
        descWrites[0].pBufferInfo = &frameUniformBufferInfo;
        descWrites[0].pTexelBufferView = nullptr;
        descWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descWrites[1].pNext = nullptr;
        descWrites[1].dstSet = m_frameDescriptorSets[i]->GetHandle();
        descWrites[1].dstBinding = 1;
        descWrites[1].dstArrayElement = 0;
        descWrites[1].descriptorCount = 1;
        descWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descWrites[1].pImageInfo = nullptr;
        VkDescriptorBufferInfo meshUniformBufferInfo =
        {   // buffer, offset, range
            VkDescriptorBufferInfo{ m_uniformBuffers[i]->GetHandle(), 0, sizeof(MeshUniforms) }
        };
        descWrites[1].pBufferInfo = &meshUniformBufferInfo;
        descWrites[1].pTexelBufferView = nullptr;
        m_frameDescriptorSets[i]->Update(descWrites);
    }

    m_meshDescriptorSetLayout = m_device->CreateDescriptorSetLayout(
        std::array{ // layout bindings
        // binding, type, count, stageFlags, pImmutableSamplers
        VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},   // baseColor
        });

    CreateSamplers();

    m_pipelineLayout = m_device->CreatePipelineLayout(
        std::array{
            m_frameDescriptorSetLayout.get(),
            m_meshDescriptorSetLayout.get(),
            m_samplerSetLayout.get(),
        }
    );

    int windowWidth = 0;
    int windowHeight = 0;
    m_window->GetSize(&windowWidth, &windowHeight);
    m_viewports.resize(1);
    m_viewports[0].x = 0;
    m_viewports[0].y = 0;
    m_viewports[0].width = float(windowWidth);
    m_viewports[0].height = float(windowHeight);
    m_viewports[0].minDepth = 0.0f;
    m_viewports[0].maxDepth = 1.0f;
    m_scissors.resize(1);
    m_scissors[0].offset.x = 0;
    m_scissors[0].offset.y = 0;
    m_scissors[0].extent.width = windowWidth;
    m_scissors[0].extent.height = windowHeight;

    m_scene = MakeRefCounted<VulkanScene>(m_device);
}

VulkanRenderer::~VulkanRenderer()
{
}

bool VulkanRenderer::Import3DModel(const Path& filePath)
{
    if (m_scene->Import(filePath))
    {
        // create pipelines
        for (uint32_t i = 0; i < m_scene->m_meshes.size(); ++i)
        {
            VulkanMesh* mesh = m_scene->m_meshes[i].get();
            CreateSolidWireframePipeline(mesh);

            if (!mesh->m_descriptorSet)
            {
                mesh->m_descriptorSet =
                    m_descriptorPool->Allocate(m_meshDescriptorSetLayout.get());
            }

            VulkanMaterial* material = mesh->m_material.get();
            if (material->m_baseColorTexture)
            {
                mesh->m_descriptorSet->UpdateImages(
                    0, 0, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                    std::array{ material->m_baseColorTexture->image->GetDefaultView() },
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            }
        }

        VulkanCamera* camera = m_scene->m_camera.get();

        int drawWidth = 0;
        int drawHeight = 0;
        m_window->GetDrawableSize(&drawWidth, &drawHeight);

        BoundingBox sceneBox = m_scene->GetBoundingBox();
        glm::vec3 sceneDiagonal = sceneBox.Diagonal();
        float sceneDiagonalLength = glm::length(sceneDiagonal);
        camera->m_aspectRatio = float(drawWidth) / float(drawHeight);

        // @TODO: how to calculate best clipNear and clipFar?
        camera->m_zNear = sceneDiagonalLength / 1000.0f;
        camera->m_zFar = sceneDiagonalLength * 4.0f;

        return true;
    }
    else
    {
        return false;
    }
}

void VulkanRenderer::CreateSolidWireframePipeline(VulkanMesh* mesh)
{
    std::vector<ShaderMacro> shaderMacros = GetShaderMacros(mesh);
    std::string pipelineKey;
    pipelineKey.reserve(64);
    for (const ShaderMacro& macro : shaderMacros)
    {
        pipelineKey += macro.m_name;
        if (!macro.m_definition.empty())
        {
            pipelineKey.push_back('=');
            pipelineKey += macro.m_definition;
        }
        pipelineKey.push_back(';');
    }

    auto iter = m_solidWireframePipelines.find(pipelineKey);
    if (iter != m_solidWireframePipelines.end())
    {
        mesh->m_pipeline = iter->second;
    }
    else
    {
        VulkanGraphicsPipelineCreateInfo pipelineInfo = {};

        pipelineInfo.m_shaders =
        {
            m_device->CreateShaderFromFile(
                VK_SHADER_STAGE_VERTEX_BIT, m_shaderSourceDir + "SolidWireframe.vert", "main", shaderMacros),
            m_device->CreateShaderFromFile(
                VK_SHADER_STAGE_FRAGMENT_BIT, m_shaderSourceDir + "SolidWireframe.frag", "main", shaderMacros),
        };
        SetVertexInputState(pipelineInfo, mesh);
        pipelineInfo.m_inputAssemblyState;
        pipelineInfo.m_tessellationState;
        pipelineInfo.m_viewportCount = 1;
        pipelineInfo.m_scissorCount = 1;
        pipelineInfo.m_rasterizationState;
        pipelineInfo.m_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineInfo.m_depthStencilState;
        pipelineInfo.m_depthStencilState.depthTestEnable = VK_TRUE;
        pipelineInfo.m_depthStencilState.depthWriteEnable = VK_TRUE;
        VulkanPipelineColorBlendAttachmentState colorBlendState = {};
        pipelineInfo.m_colorBlendState.attachments.push_back(colorBlendState);
        pipelineInfo.m_layout = m_pipelineLayout.get();
        pipelineInfo.m_renderPass = m_window->GetDefaultRenderPass();
        pipelineInfo.m_subpass = 0;
        pipelineInfo.m_basePipeline;
        pipelineInfo.m_basePipelineIndex = 0;
        Ref<VulkanGraphicsPipeline> pipeline = m_device->CreateGraphicsPipeline(pipelineInfo);
        m_solidWireframePipelines[pipelineKey] = pipeline;
        mesh->m_pipeline = pipeline;
    }
}

std::vector<ShaderMacro> VulkanRenderer::GetShaderMacros(VulkanMesh* mesh)
{
    std::vector<ShaderMacro> shaderMacros;
    if (mesh->m_hasPosition)
    {
        shaderMacros.push_back(ShaderMacro("HAS_POSITION"));
    }
    if (mesh->m_hasNormal)
    {
        shaderMacros.push_back(ShaderMacro("HAS_NORMAL"));
    }
    if (mesh->m_hasTangent)
    {
        shaderMacros.push_back(ShaderMacro("HAS_TANGENT"));
    }
    if (mesh->m_numUVChannels > 0)
    {
        shaderMacros.push_back(ShaderMacro("NUM_UV_CHANNELS", mesh->m_numUVChannels));
    }
    if (mesh->m_hasColor)
    {
        shaderMacros.push_back(ShaderMacro("HAS_COLOR"));
    }
    if (mesh->m_material->m_baseColorTexture)
    {
        shaderMacros.push_back(ShaderMacro("HAS_BASE_COLOR_TEXTURE"));
    }
    return shaderMacros;
}

void VulkanRenderer::SetVertexInputState(VulkanGraphicsPipelineCreateInfo& pipelineInfo, VulkanMesh* mesh)
{
    pipelineInfo.AddVertexBinding(0, mesh->m_vertexStride);
    uint32_t attribOffset = 0;
    if (mesh->m_hasPosition)
    {
        pipelineInfo.AddVertexAttribute(0, 0, VK_FORMAT_R32G32B32_SFLOAT, attribOffset);
        attribOffset += sizeof(glm::vec3);
    }
    if (mesh->m_hasNormal)
    {
        pipelineInfo.AddVertexAttribute(1, 0, VK_FORMAT_R32G32B32_SFLOAT, attribOffset);
        attribOffset += sizeof(glm::vec3);
    }
    if (mesh->m_hasTangent)
    {
        pipelineInfo.AddVertexAttribute(2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, attribOffset);
        attribOffset += sizeof(glm::vec4);
    }
    if (mesh->m_hasColor)
    { // vertex color
        pipelineInfo.AddVertexAttribute(3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, attribOffset);
        attribOffset += sizeof(glm::vec4);
    }
    uint32_t texCoordLocation = 4; // texCoord started at location=4
    for (uint32_t texCoordIndex = 0; texCoordIndex < mesh->m_numUVChannels; texCoordIndex++)
    {
        pipelineInfo.AddVertexAttribute(texCoordLocation++, 0, VK_FORMAT_R32G32_SFLOAT, attribOffset);
        attribOffset += sizeof(glm::vec2);
    }
}

void VulkanRenderer::Reset()
{
    m_scene = MakeRefCounted<VulkanScene>(m_device);

    m_uniformData.clear();
    m_uniformBuffers.clear();
    m_solidWireframePipelines.clear();
}

void VulkanRenderer::Resize(uint32_t width, uint32_t height)
{
    m_scene->m_camera->m_aspectRatio = float(width) / float(height);
}

void VulkanRenderer::Render(float deltaTime)
{
    uint32_t swapchainImageIndex = m_window->GetSwapchain()->GetCurrentImageIndex();

    VulkanCamera* camera = m_scene->m_camera.get();

    m_uniformOffset = 0;
    FrameUniforms frameUniforms = {};
    glm::mat4 correctionMatrix = {
        +1.0, +0.0, +0.0, +0.0,
        +0.0, -1.0, +0.0, +0.0,
        +0.0, +0.0, +0.5, +0.5,
        +0.0, +0.0, +0.0, +1.0,
    };
    frameUniforms.viewProjectionMatrix = correctionMatrix *
        camera->GetProjectionMatrix() * camera->GetViewMatrix();
    WriteUniforms(&frameUniforms, sizeof(frameUniforms));

    if (m_scene)
    {
        RenderNodes(m_scene->m_rootNode.get(), glm::identity<glm::mat4>());
    }
}

void VulkanRenderer::RenderNodes(VulkanSceneNode* node, glm::mat4 transform)
{
    transform = transform * node->m_transform;

    // @TODO: render with multi-thread
    VulkanCommandBuffer* cmdBuffer = m_window->GetCommandBuffer();
    VulkanSwapchain* swapchain = m_window->GetSwapchain();

    for (uint32_t i = 0; i < node->m_meshes.size(); i++)
    {
        VulkanMesh* mesh = node->m_meshes[i].get();
        MeshUniforms meshUniforms = {};
        meshUniforms.modelToWorld = transform;
        uint32_t meshUniformOffset = WriteUniforms(&meshUniforms, sizeof(meshUniforms));

        VulkanPipeline* pipeline = mesh->m_pipeline.get();
        cmdBuffer->BindPipeline(pipeline);
        cmdBuffer->BindDescriptorSets(pipeline, m_pipelineLayout.get(), 0,
            std::array{ // descriptor sets
                m_frameDescriptorSets[swapchain->GetCurrentImageIndex()].get(),
                mesh->m_descriptorSet.get(),
                m_samplerSet.get(),
            },
            std::array{ // dynamic offsets
                0u,  // set = 0, binding = 0: FrameUniforms
                meshUniformOffset,  // set = 0, binding = 1: MeshUniforms
            }
        );
        cmdBuffer->BindVertexBuffers(0, std::array{ mesh->m_vertexBuffer.get() }, std::array{ mesh->m_vertexBufferOffset });
        if (mesh->m_indexBuffer)
        {
            cmdBuffer->BindIndexBuffer(mesh->m_indexBuffer.get(), mesh->m_indexBufferOffset, VK_INDEX_TYPE_UINT32);
        }

        cmdBuffer->SetViewports(m_viewports);
        cmdBuffer->SetScissors(m_scissors);

        if (mesh->m_indexBuffer)
        {
            cmdBuffer->DrawIndexed(mesh->GetIndexCount(), 1, 0, 0, 0);
        }
        else
        {
            cmdBuffer->Draw(mesh->GetVertexCount(), 1, 0, 0);
        }
    }

    // traversal the scene nodes recursively
    for (auto& childNode : node->m_children)
    {
        RenderNodes(childNode.get(), transform);
    }
}

void VulkanRenderer::CreateSamplers()
{
    VkSamplerCreateInfo samplerCreateInfo = {};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext = nullptr;
    samplerCreateInfo.flags = 0;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16.0f;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.0f;
    samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    m_sampler = m_device->CreatSampler(samplerCreateInfo);

    m_samplerDescriptorPool = m_device->CreateDescriptorPool(1,
        std::array{
            VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,                   32 },
        });
    m_samplerSetLayout = m_device->CreateDescriptorSetLayout(
        std::array{ // layout bindings
        // binding, type, count, stageFlags, pImmutableSamplers
        VkDescriptorSetLayoutBinding{ 0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_ALL, nullptr},
        }
    );

    m_samplerSet = m_samplerDescriptorPool->Allocate(m_samplerSetLayout.get());
    VkWriteDescriptorSet descWrites[1] = {};
    descWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descWrites[0].pNext = nullptr;
    descWrites[0].dstSet = m_samplerSet->GetHandle();
    descWrites[0].dstBinding = 0;
    descWrites[0].dstArrayElement = 0;
    descWrites[0].descriptorCount = 1;
    descWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    VkDescriptorImageInfo samplerInfo = {};
    samplerInfo.sampler = m_sampler->GetHandle();
    samplerInfo.imageView;
    samplerInfo.imageLayout;
    descWrites[0].pImageInfo = &samplerInfo;
    descWrites[0].pBufferInfo = nullptr;
    descWrites[0].pTexelBufferView = nullptr;
    m_samplerSet->Update(descWrites);
}

uint32_t VulkanRenderer::WriteUniforms(void* data, VkDeviceSize dataSize)
{
    uint32_t currentUniformOffset = static_cast<uint32_t>(m_uniformOffset);

    uint32_t swapchainImageIndex = m_window->GetSwapchain()->GetCurrentImageIndex();
    memcpy(m_uniformData[swapchainImageIndex] + m_uniformOffset, data, dataSize);
    VkDeviceSize offsetAlignment = m_device->GetPhysicalDevice()->GetProperties().limits.minUniformBufferOffsetAlignment;
    m_uniformOffset += RoundUpToMultiple(dataSize, offsetAlignment);

    return currentUniformOffset;
}
