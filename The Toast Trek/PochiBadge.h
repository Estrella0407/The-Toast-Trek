#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Font;
class Pochi;

// Top-left overworld HUD: the pochiState badge
// red HP, blue DEF, green ATK
class PochiBadge {
public:
    explicit PochiBadge(IDirect3DDevice9* device);
    ~PochiBadge();

    void Draw(LPD3DXSPRITE brush, const Pochi& stats);

private:
    IDirect3DTexture9* badgeTex;   // Assets/UI/pochiStateFull.png
    Font* valueFont;
    IDirect3DTexture9* whiteTex;   // Wash quad + backing plate behind the value text
};
