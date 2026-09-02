#include "PochiBadge.h"
#include "Font.h"
#include "Pochi.h"
#include "UiFill.h"
#include <algorithm>
#include <string>

namespace {
    // --- On-screen layout (pixels) ------------------------
    constexpr float kBadgeScale = 2.2f;
    constexpr float kBadgeX = -30.0f;
    constexpr float kBadgeY = -97.0f;   // pulls the badge's empty top margin off-screen

    // kFillL is the first x of colour (just pastnthe medallion)
    // fillR is the last x of colour
    // red   x 47..97  rows 57-59
    // blue  x 47..91  rows 62-64
    // green x 47..85  rows 67-69
    constexpr int kArtW = 128, kArtH = 128;
    constexpr int kFillL = 47;

    struct Stripe { int y0, y1, fillR; };
    constexpr Stripe kHp  { 57, 60, 97 };   // red
    constexpr Stripe kDef { 62, 65, 91 };   // blue
    constexpr Stripe kAtk { 67, 70, 85 };   // green

    // Darken the unfilled tail of each bar
    const D3DCOLOR kEmptyWash = D3DCOLOR_ARGB(255, 58, 36, 24);

    // --- Value readout -------------------------------------------------
    constexpr float kValueX = 230.0f;
    constexpr float kRowHpY = 18.0f;
    constexpr float kRowDefY = 42.0f;
    constexpr float kRowAtkY = 66.0f;

    // Plate / edge / shadow: ui::kPlate, ui::kPlateEdge, ui::kShadow
    constexpr float kPlateX = 220.0f, kPlateY = 12.0f, kPlateW = 128.0f, kPlateH = 80.0f;    

    // The shared palette so every FillRect plate in the game matches
    const D3DCOLOR kHpCol = D3DCOLOR_XRGB(255, 138, 118);
    const D3DCOLOR kDefCol = D3DCOLOR_XRGB(150, 205, 255);
    const D3DCOLOR kAtkCol = D3DCOLOR_XRGB(160, 240, 150);
}

PochiBadge::PochiBadge(IDirect3DDevice9* device)
    : badgeTex(NULL), valueFont(NULL), whiteTex(NULL) {
    badgeTex = ui::LoadTexture(device, "Assets/UI/pochiStateFull.png", kArtW, kArtH);
    valueFont = new Font(device, 0.0f, 0.0f, 120, 20, 16, "Arial");
    whiteTex = ui::MakeWhiteTexture(device);
}

PochiBadge::~PochiBadge() {
    if (badgeTex != NULL) badgeTex->Release();
    delete valueFont;
    if (whiteTex != NULL) whiteTex->Release();
}

void PochiBadge::Draw(LPD3DXSPRITE brush, const Pochi& stats) {
    const int hp = stats.GetHealth();
    const int hpMax = stats.GetMaxHealth();
    const int def = stats.GetArmor();
    const int defMax = stats.GetMaxArmor();
    const int atk = stats.GetAttackDamage();
    const int level = stats.GetLevel();

    const float hpRatio = hpMax > 0 ? (float)hp / hpMax : 0.0f;
    const float defRatio = defMax > 0 ? (float)def / defMax : 0.0f;
    const float atkRatio = level / 3.0f;    // level tops out at 3

    const float S = kBadgeScale;

    // Base: the finished badge (wood flag + medallion + all bars full)
    if (badgeTex != NULL) {
        ui::DrawTexture(brush, badgeTex, kArtW, kArtH, kBadgeX, kBadgeY, S, S);
    }

    // Darken each bar from its fill point to its end
    // so only the filled part shows colour
    auto Drain = [&](const Stripe& s, float ratio) {
        if (whiteTex == NULL) return;
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        const float xFill = kFillL + ratio * (float)(s.fillR - kFillL);
        if (xFill >= (float)s.fillR) return;
        ui::FillRect(brush, whiteTex,
            kBadgeX + xFill * S, kBadgeY + s.y0 * S,
            ((float)s.fillR - xFill) * S, (float)(s.y1 - s.y0) * S, kEmptyWash);
    };
    Drain(kHp, hpRatio);
    Drain(kDef, defRatio);
    Drain(kAtk, atkRatio);

    if (valueFont != NULL) {
        // Dark plate + gold edge behind the readout
        ui::FillRect(brush, whiteTex, kPlateX - 1.0f, kPlateY - 1.0f, kPlateW + 2.0f, kPlateH + 2.0f, ui::kPlateEdge);
        ui::FillRect(brush, whiteTex, kPlateX, kPlateY, kPlateW, kPlateH, ui::kPlate);

        const std::string hpText = "HP  " + std::to_string(hp) + " / " + std::to_string(hpMax);
        const std::string defText = "DEF " + std::to_string(def) + " / " + std::to_string(defMax);
        const std::string atkText = "ATK " + std::to_string(atk);

        auto Row = [&](const char* s, float y, D3DCOLOR col) {
            valueFont->Draw(s, kValueX + 1.0f, y + 1.0f, ui::kShadow, brush);   // drop shadow
            valueFont->Draw(s, kValueX, y, col, brush);
            };
        Row(hpText.c_str(), kRowHpY, kHpCol);
        Row(defText.c_str(), kRowDefY, kDefCol);
        Row(atkText.c_str(), kRowAtkY, kAtkCol);
    }
}
