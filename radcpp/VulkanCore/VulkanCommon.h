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

class VulkanFormat
{
public:
    VulkanFormat(VkFormat format) :
        m_format(format)
    {
    }
    ~VulkanFormat()
    {
    }

    operator VkFormat() const { return m_format; }

    // Numeric
    // Formats with more then one numeric type (VK_FORMAT_D16_UNORM_S8_UINT) will return false
    bool IsUNORM() const;
    bool IsSNORM() const;
    bool IsUSCALED() const;
    bool IsSSCALED() const;
    bool IsUINT() const;
    bool IsSINT() const;
    bool IsSRGB() const;
    bool IsSFLOAT() const;
    bool IsUFLOAT() const;

    // Types from "Interpretation of Numeric Format" table (OpTypeFloat vs OpTypeInt)
    bool IsSampledInt() const;
    bool IsSampledFloat() const;

    bool IsCompressed_ASTC_HDR() const;
    bool IsCompressed_ASTC_LDR() const;
    bool IsCompressed_BC() const;
    bool IsCompressed_EAC() const;
    bool IsCompressed_ETC2() const;
    bool IsCompressed_PVRTC() const;
    bool IsCompressed() const;

    VkImageAspectFlags GetAspectFlags() const;
    bool IsDepthStencil() const;
    bool IsDepthOnly() const;
    bool IsStencilOnly() const;
    bool HasDepth() const;
    bool HasStencil() const;
    uint32_t GetDepthSizeInBits() const;
    uint32_t GetStencilSizeInBits() const;

    bool IsPacked() const;

    bool RequiresYcbcrConversion() const;
    bool IsXChromaSubsampled() const;
    bool IsYChromaSubsampled() const;

    bool IsSinglePlane_422() const;
    uint32_t GetPlaneCount() const;
    bool IsIsMultiplane() const;
    VkFormat GetMultiplaneCompatibleFormat(VkImageAspectFlags plane_aspect) const;
    VkExtent2D GetMultiplaneExtentDivisors(VkImageAspectFlags plane_aspect) const;

    size_t GetComponentCount() const;
    VkExtent3D GetTexelBlockExtent() const;
    size_t GetElementSizeInBytes(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) const;
    double GetTexelSizeInBytes(VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT) const;

    bool IsUndefined() const;
    bool IsBlockedImage() const;
    bool IsColor() const;

private:
    VkFormat m_format;

}; // class VulkanFormat

#endif // VULKAN_COMMON_H