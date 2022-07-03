#include "radcpp/Common/Common.h"
#include "radcpp/Common/Log.h"

#include "HelloWorld.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);

    bool enableValidationLayer = true;

    Ref<VulkanInstance> instance = VulkanInstance::Create(
        "HelloWorld", VK_MAKE_VERSION(0, 0, 0),
        enableValidationLayer
    );

    HelloWorld helloVulkan(instance);
    helloVulkan.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1600, 900, 0);
    helloVulkan.Init();

    return app.Run();
}