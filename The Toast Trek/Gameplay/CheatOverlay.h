#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Font;

// The red "CHEAT MODE" plate drawn upper-right over every scene while the
// developer cheat switch (F5) is on. Owns its own font + plate texture.
class CheatOverlay {
private:
    Font* font;
    IDirect3DTexture9* plateTex;

public:
    CheatOverlay();
    ~CheatOverlay();

    void Load(IDirect3DDevice9* device);

    // Draws only when Cheats::enabled. Call inside an open sprite batch.
    void Draw(LPD3DXSPRITE brush);
};
