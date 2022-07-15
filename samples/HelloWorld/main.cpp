#include "HelloWorld.h"

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

    HelloWorld helloVulkan(instance);
    helloVulkan.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1600, 900, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    helloVulkan.Init();

    return app.Run();
}