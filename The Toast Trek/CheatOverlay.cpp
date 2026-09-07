#include "CheatOverlay.h"
#include "Font.h"
#include "Cheats.h"
#include "UiFill.h"

CheatOverlay::CheatOverlay()
    : font(nullptr)
{
}

CheatOverlay::~CheatOverlay()
{
    delete font;
}

void CheatOverlay::Load(IDirect3DDevice9* device)
{
    // Wide rect so the banner still renders when drawn far to the right
    // (Font::Draw(x,y,...) inverts its rect once x passes the width)
    font = new Font(device, 0.0f, 0.0f, 1600, 30, 18, "Arial");
}

void CheatOverlay::Draw(LPD3DXSPRITE brush)
{
    if (!Cheats::enabled || font == nullptr) return;

    const char* txt = "CHEAT MODE";
    const float pw = 118.0f, ph = 24.0f;
    const float px = 1280.0f - pw - 12.0f, py = 12.0f;

    ui::FillRect(brush, px - 1.0f, py - 1.0f, pw + 2.0f, ph + 2.0f, ui::kPlateEdge);
    ui::FillRect(brush, px, py, pw, ph, ui::kPlate);

    font->Draw(txt, px + 15.0f, py + 4.0f, ui::kShadow, brush);
    font->Draw(txt, px + 14.0f, py + 3.0f, D3DCOLOR_XRGB(255, 120, 120), brush);
}
