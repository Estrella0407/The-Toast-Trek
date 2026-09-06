#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"
#include "Font.h"

class MainMenu {
private:
    Sprite* pochi;
    Font* titleFont;
    Font* promptFont;
    int animCounter;
    int animDelay;

public:
    MainMenu(IDirect3DDevice9* d3dDevice, Sprite* sharedPochi);
    ~MainMenu();

    void Update();
    void Draw(LPD3DXSPRITE brush);
};
