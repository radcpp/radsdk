#ifndef VULKAN_COMMON_H
#define VULKAN_COMMON_H

// Loading Vulkan entrypoints directly from the driver can increase performance by skipping loader dispatch overhead.
// Each VulkanDevice has its own VolkDeviceTable.
#define VK_NO_PROTOTYPES 1

// Enable Vulkan beta extensions
#define VK_ENABLE_BETA_EXTENSIONS 1

// If you want VMA to fetch pointers to Vulkan functions dynamically using vkGetInstanceProcAddr, vkGetDeviceProcAddr (this is the option presented in the example below):
// Define VMA_STATIC_VULKAN_FUNCTIONS to 0, VMA_DYNAMIC_VULKAN_FUNCTIONS to 1.
// Provide pointers to these two functions via VmaVulkanFunctions::vkGetInstanceProcAddr, VmaVulkanFunctions::vkGetDeviceProcAddr.
// The library will fetch pointers to all other functions it needs internally.
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include "volk.h"
#include "vulkan/vk_enum_string_helper.h"

// VulkanMemoryAllocator (VMA) library manages memory for VulkanBuffer and VulkanImage.
#include "vk_mem_alloc.h"

// Common headers
#include "radcpp/Common/Containers.h"
#include "radcpp/Common/Exception.h"
#include "radcpp/Common/Log.h"
#include "radcpp/Common/Memory.h"
#include "radcpp/Common/String.h"

// Base of all Vulkan classes
class VulkanObject : public RefCounted<VulkanObject>
{
public:
    VulkanObject() {}
    virtual ~VulkanObject() {}
}; // class VulkanObject

class VulkanError : public std::exception
{
public:
    VulkanError(VkResult result) : m_result(result) {}
    char const* what() const override { return string_VkResult(m_result); }
private:
    VkResult m_result;
}; // class VulkanError

// Check Vulkan return code and throw VulkanError if result<0
void ReportVulkanError(VkResult result, const char* function, const char* file, uint32_t line);
#define VK_CHECK(VulkanCall) { const VkResult _result = VulkanCall; if (_result < 0) { ReportVulkanError(_result, #VulkanCall, __FILE__, __LINE__); } }

std::string GetVulkanVersionString(uint32_t versionNumber);

#endif // VULKAN_COMMON_H