#include "radcpp/Common/Common.h"
#include "radcpp/Common/Log.h"

#include "WindowTest.h"

int main(int argc, char* argv[])
{
    Application app(argc, argv);

    WindowTest windowTest;
    windowTest.Create("Hello, World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600, 0);

    return app.Run();
}