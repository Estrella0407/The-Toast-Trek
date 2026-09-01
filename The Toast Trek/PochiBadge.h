#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Font;
class Pochi;

// Top-left overworld HUD: the pochiState badge, whose flag carries three
// stacked bars - red HP, blue DEF, green ATK. Drawn as pochiStateFull.png
// (the finished badge, all bars full) with the un-filled tail of each bar
// washed dark to that stat's ratio. A small value readout sits beside it.
class PochiBadge {
public:
    explicit PochiBadge(IDirect3DDevice9* device);
    ~PochiBadge();

    void Draw(LPD3DXSPRITE brush, const Pochi& stats);

private:
    IDirect3DTexture9* badgeTex;   // Assets/UI/pochiStateFull.png (128x128)
    Font* valueFont;
    IDirect3DTexture9* whiteTex;   // wash quad + backing plate behind the value text
};
