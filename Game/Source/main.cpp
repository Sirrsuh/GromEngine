#include <Game/Game.h>

int main()
{
    grom::Game game;
    if (!game.Initialize())
        return 1;
    game.Run();
    game.Shutdown();
    return 0;
}
