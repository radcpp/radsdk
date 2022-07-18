#include "OpenCompute.h"

std::vector<Ref<ClPlatform>> ClPlatform::GetPlatforms()
{
    std::vector<Ref<ClPlatform>> platforms;
    std::vector<cl_platform_id> platformIds;
    cl_uint platformCount = 0;
    clGetPlatformIDs(0, nullptr, &platformCount);
    if (platformCount > 0)
    {
        platformIds.resize(platformCount);
        clGetPlatformIDs(platformCount, platformIds.data(), &platformCount);
        platforms.resize(platformCount);
        for (uint32_t i = 0; i < platformCount; ++i)
        {
            platforms[i] = MakeRefCounted<ClPlatform>(platformIds[i]);
        }
    }

    return platforms;
}

ClPlatform::ClPlatform(cl_platform_id platformId) :
    m_platformId(platformId)
{
    m_name = GetPlatformInfoString(CL_PLATFORM_NAME);
    m_vendor = GetPlatformInfoString(CL_PLATFORM_VENDOR);
    m_version = GetPlatformInfoString(CL_PLATFORM_VERSION);

    std::vector<cl_device_id> deviceIds;
    cl_uint deviceCount = 0;
    clGetDeviceIDs(m_platformId, CL_DEVICE_TYPE_ALL, 0, nullptr, &deviceCount);
    if (deviceCount > 0)
    {
        deviceIds.resize(deviceCount);
        clGetDeviceIDs(m_platformId, CL_DEVICE_TYPE_ALL, deviceCount, deviceIds.data(), &deviceCount);
        m_devices.resize(deviceCount);
        for (uint32_t i = 0; i < deviceCount; ++i)
        {
            m_devices[i] = MakeRefCounted<ClDevice>(deviceIds[i]);
        }
    }
}

ClPlatform::~ClPlatform()
{
}

std::string ClPlatform::GetPlatformInfoString(cl_platform_info paramName) const
{
    std::string info;
    size_t size = 0;
    cl_int result = clGetPlatformInfo(m_platformId, paramName, 0, nullptr, &size);
    if ((result == CL_SUCCESS) && (size > 0))
    {
        info.resize(size);
        result = clGetPlatformInfo(m_platformId, paramName, size, info.data(), nullptr);
        if (result == CL_SUCCESS)
        {
            return info;
        }
    }
    return std::string();
}

ClDevice::ClDevice(cl_device_id deviceId) :
    m_deviceId(deviceId)
{
    m_name = GetDeviceInfoString(CL_DEVICE_NAME);
    m_extensionString = GetDeviceInfoString(CL_DEVICE_EXTENSIONS);
}

ClDevice::~ClDevice()
{
}

std::string ClDevice::GetDeviceInfoString(cl_device_info paramName) const
{
    std::string info;
    size_t size = 0;
    cl_int result = clGetDeviceInfo(m_deviceId, paramName, 0, nullptr, &size);
    if ((result == CL_SUCCESS) && (size > 0))
    {
        info.resize(size);
        result = clGetDeviceInfo(m_deviceId, paramName, size, info.data(), nullptr);
        if (result == CL_SUCCESS)
        {
            return info;
        }
    }
    return std::string();
}

const char* ClDevice::GetDeviceTypeString(cl_device_type type)
{
    switch (type)
    {
    case CL_DEVICE_TYPE_DEFAULT: return "Default";
    case CL_DEVICE_TYPE_CPU: return "CPU";
    case CL_DEVICE_TYPE_GPU: return "GPU";
    case CL_DEVICE_TYPE_CUSTOM: return "Custom";
    }
    return "Unknown";
}

bool ClDevice::SupportsExtension(std::string_view extensionName) const
{
    return (m_extensionString.find(extensionName) != std::string::npos);
}
