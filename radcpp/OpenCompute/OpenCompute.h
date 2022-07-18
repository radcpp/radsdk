#ifndef OPEN_COMPUTE_H
#define OPEN_COMPUTE_H

#include "radcpp/Common/Memory.h"
#include "radcpp/Common/String.h"
#include "CL/cl.h"

class ClPlatform;
class ClDevice;

class ClPlatform : public RefCounted<ClPlatform>
{
public:
    static std::vector<Ref<ClPlatform>> GetPlatforms();

    ClPlatform(cl_platform_id platformId);
    ~ClPlatform();

    std::string GetPlatformInfoString(cl_platform_info paramName) const;

    const char* GetName() const { return m_name.c_str(); }
    const char* GetVendor() const { return m_vendor.c_str(); }
    const char* GetVersion() const { return m_version.c_str(); }

    std::vector<Ref<ClDevice>> GetDevices() { return m_devices; }

private:
    cl_platform_id m_platformId = 0;
    std::string m_name;
    std::string m_vendor;
    std::string m_version;

    std::vector<Ref<ClDevice>> m_devices;

}; // ClPlatform

class ClDevice : public RefCounted<ClDevice>
{
public:
    ClDevice(cl_device_id deviceId);
    ~ClDevice();

    std::string GetDeviceInfoString(cl_device_info paramName) const;
    static const char* GetDeviceTypeString(cl_device_type type);

    const char* GetName() const { return m_name.c_str(); }
    const char* GetExtensionString() const { return m_extensionString.c_str(); }
    bool SupportsExtension(std::string_view extensionName) const;

private:
    cl_device_id m_deviceId = 0;
    std::string m_name;
    std::string m_extensionString;

}; // class ClDevice

#endif // OPEN_COMPUTE_H