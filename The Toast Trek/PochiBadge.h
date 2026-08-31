#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Font;
class Pochi;
class FrameBar;

// Top-left overworld HUD: the pochiState badge, whose flag carries three
// stacked fill lines - red HP, blue DEF, green ATK. Each line is its own
// vertical strip PNG (so HP can be full while DEF/ATK are low), drawn on
// top of the badge base at the same spot. A small value readout sits
// beside it.
class PochiBadge {
public:
    explicit PochiBadge(IDirect3DDevice9* device);
    ~PochiBadge();

    void Draw(LPD3DXSPRITE brush, const Pochi& stats);

private:
    FrameBar* hpBadge;   // badge_hp_strip.png  - badge base + red line;  frame = health 0..9
    FrameBar* defLine;   // badge_def_strip.png - blue line only;         frame = armor 0..3
    FrameBar* atkLine;   // badge_atk_strip.png - green line only;        frame = level 0..3
    Font* valueFont;
};
