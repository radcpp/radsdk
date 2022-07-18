#include "HelloWorld.h"
#include "radcpp/OpenCompute/OpenCompute.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);

#ifdef _DEBUG
    bool enableValidationLayer = true;
#else
    bool enableValidationLayer = false;
#endif
    Ref<VulkanInstance> instance = VulkanInstance::Create(
        "HelloWorld", VK_MAKE_VERSION(0, 0, 0),
        enableValidationLayer
    );

    auto clplatforms = ClPlatform::GetPlatforms();
    for (const auto& platform : clplatforms)
    {
        LogPrint("OpenCompute", LogLevel::Info, "Platform: %s (%s)", platform->GetName(), platform->GetVersion());
        auto devices = platform->GetDevices();
        for (uint32_t i = 0; i < devices.size(); ++i)
        {
            LogPrint("OpenCompute", LogLevel::Info, "Device#%u: %s", i, devices[i]->GetName());
            LogPrint("OpenCompute", LogLevel::Info, "Version: %s", devices[i]->GetDeviceInfoString(CL_DEVICE_VERSION).c_str());
            LogPrint("OpenCompute", LogLevel::Info, "Profile: %s", devices[i]->GetDeviceInfoString(CL_DEVICE_PROFILE).c_str());
            LogPrint("OpenCompute", LogLevel::Info, "Driver: %s", devices[i]->GetDeviceInfoString(CL_DRIVER_VERSION).c_str());
            auto extensions = StrSplit(devices[i]->GetExtensionString(), " ");
            for (const std::string& extension : extensions)
            {
                LogPrint("OpenCompute", LogLevel::Info, "Extension: %s", extension.c_str());
            }
        }
    }

    HelloWorld helloVulkan(instance);
    helloVulkan.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1600, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    helloVulkan.Init();

    return app.Run();
}