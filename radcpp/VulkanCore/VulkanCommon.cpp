#include "radcpp/VulkanCore/VulkanCommon.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

void ReportVulkanError(VkResult result, const char* function, const char* file, uint32_t line)
{
    LogPrint("Vulkan", LogLevel::Error, "%s failed with VkResult=%s(%d).",
        function, string_VkResult(result), result, file, line);
    throw VulkanError(result);
}

std::string GetVulkanVersionString(uint32_t versionNumber)
{
    return StrFormat("%u.%u.%u",
        VK_VERSION_MAJOR(versionNumber),
        VK_VERSION_MINOR(versionNumber),
        VK_VERSION_PATCH(versionNumber));
}
