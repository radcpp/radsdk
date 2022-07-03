#include "VulkanDescriptorSet.h"
#include "VulkanDevice.h"
#include "VulkanDescriptorPool.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanSampler.h"

VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(Ref<VulkanDevice> device, const VkDescriptorSetLayoutCreateInfo & createInfo) :
    m_device(std::move(device))
{
    VK_CHECK(m_device->GetFunctionTable()->
        vkCreateDescriptorSetLayout(m_device->GetHandle(), &createInfo, nullptr, &m_handle));
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout()
{
    m_device->GetFunctionTable()->
        vkDestroyDescriptorSetLayout(m_device->GetHandle(), m_handle, nullptr);
}

VulkanDescriptorSet::VulkanDescriptorSet(
    Ref<VulkanDevice> device, Ref<VulkanDescriptorPool> descriptorPool, Ref<VulkanDescriptorSetLayout> layout) :
    m_device(std::move(device)),
    m_descriptorPool(std::move(descriptorPool)),
    m_layout(std::move(layout))
{
    VkDescriptorSetLayout layoutHandle = m_layout->GetHandle();

    VkDescriptorSetAllocateInfo allocateInfo = {};
    allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.descriptorPool = m_descriptorPool->GetHandle();
    allocateInfo.descriptorSetCount = 1;
    allocateInfo.pSetLayouts = &layoutHandle;

    VK_CHECK(m_device->GetFunctionTable()->
        vkAllocateDescriptorSets(m_device->GetHandle(), &allocateInfo, &m_handle));
}

VulkanDescriptorSet::~VulkanDescriptorSet()
{
    m_device->GetFunctionTable()->
        vkFreeDescriptorSets(m_device->GetHandle(), m_descriptorPool->GetHandle(), 1, &m_handle);
}

void VulkanDescriptorSet::Update(ArrayRef<VkWriteDescriptorSet> writes, ArrayRef<VkCopyDescriptorSet> copies)
{
    m_device->GetFunctionTable()->
        vkUpdateDescriptorSets(m_device->GetHandle(),
        static_cast<uint32_t>(writes.size()), writes.data(),
        static_cast<uint32_t>(copies.size()), copies.data());
}

void VulkanDescriptorSet::UpdateBuffers(
    uint32_t binding, uint32_t arrayElement, VkDescriptorType type, ArrayRef<VulkanBuffer*> buffers)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_handle;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<uint32_t>(buffers.size());
    write.descriptorType = type;

    std::vector<VkDescriptorBufferInfo> bufferInfos(buffers.size());
    for (uint32_t i = 0; i < buffers.size(); i++)
    {
        bufferInfos[i].buffer = buffers[i]->GetHandle();
        bufferInfos[i].offset = 0;
        bufferInfos[i].range = buffers[i]->GetSize();
    }

    write.pImageInfo = nullptr;
    write.pBufferInfo = bufferInfos.data();
    write.pTexelBufferView = nullptr;

    Update(std::array{ write });
}

void VulkanDescriptorSet::UpdateImages(
    uint32_t binding, uint32_t arrayElement, VkDescriptorType type, ArrayRef<VulkanImageView*> imageViews, VkImageLayout layout)
{
    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_handle;
    write.dstBinding = binding;
    write.dstArrayElement = arrayElement;
    write.descriptorCount = static_cast<uint32_t>(imageViews.size());
    write.descriptorType = type;

    std::vector<VkDescriptorImageInfo> apiImageInfos(imageViews.size());
    for (uint32_t i = 0; i < imageViews.size(); i++)
    {
        apiImageInfos[i].sampler = VK_NULL_HANDLE;
        apiImageInfos[i].imageView = imageViews[i]->GetHandle();
        apiImageInfos[i].imageLayout = layout;
    }

    write.pImageInfo = apiImageInfos.data();
    write.pBufferInfo = nullptr;
    write.pTexelBufferView = nullptr;

    Update(std::array{ write });
}
