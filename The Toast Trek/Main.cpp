// The whole program: build the scene stack, then let the GameStateManager
// run the frame. Everything else lives in its own class.
#include "GameStateManager.h"
#include "MainMenuScene.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    GameStateManager* game = new GameStateManager();

    game->Push(CreateMainMenuScene());
    game->Init();                       // engine bring-up + top scene Initialize()

    while (game->WindowIsRunning())
    {
        game->GetInput();               // engine: input devices + cursor + sound  (code once)
        game->Update();                 // top scene HandleInput + Update           (per scene)
        game->Render();                 // engine: clear / draw / overlay / present (code once)
    }

    game->Shutdown();
    delete game;
    return 0;
}
