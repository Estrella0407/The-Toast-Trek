#include "PochiBadge.h"
#include "FrameBar.h"
#include "Font.h"
#include "Pochi.h"
#include <string>

namespace {
    // --- Frame strip layout (must match the PNGs) -----------------------
    // All three strips are 128 x 128 per frame and overlay at the same spot.
    // Frame is chosen by ratio (value / max), so a full bar always looks
    // full regardless of level: frame 0 = empty ... last frame = full.
    // Power-of-two strips: 128x2048, 16 frames of 128x128.
    constexpr int kFrameW = 128, kFrameH = 128;
    constexpr int kHpFrames = 16;
    constexpr int kDefFrames = 16;
    constexpr int kAtkFrames = 16;

    // --- On-screen layout (pixels). Tweak freely. -------------------
    constexpr float kBadgeScale = 2.2f;
    constexpr float kBadgeX = -10.0f;
    constexpr float kBadgeY = -97.0f;   // pulls the badge's empty top margin off-screen

    constexpr float kValueX = 262.0f;
    constexpr float kRowHpY = 22.0f;
    constexpr float kRowDefY = 44.0f;
    constexpr float kRowAtkY = 66.0f;

    const D3DCOLOR kHpCol = D3DCOLOR_XRGB(226, 92, 66);
    const D3DCOLOR kDefCol = D3DCOLOR_XRGB(96, 176, 230);
    const D3DCOLOR kAtkCol = D3DCOLOR_XRGB(104, 190, 96);
}

PochiBadge::PochiBadge(IDirect3DDevice9* device)
    : hpBadge(NULL), defLine(NULL), atkLine(NULL), valueFont(NULL) {
    hpBadge = new FrameBar(device, "Assets/UI/badge_hp_strip.png", kFrameW, kFrameH, kHpFrames);
    defLine = new FrameBar(device, "Assets/UI/badge_def_strip.png", kFrameW, kFrameH, kDefFrames);
    atkLine = new FrameBar(device, "Assets/UI/badge_atk_strip.png", kFrameW, kFrameH, kAtkFrames);
    valueFont = new Font(device, 0.0f, 0.0f, 90, 20, 15, "Arial");
}

PochiBadge::~PochiBadge() {
    delete hpBadge;
    delete defLine;
    delete atkLine;
    delete valueFont;
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
    const float atkRatio = level / 3.0f;   // level tops out at 3

    // Base badge + red line, then the blue and green lines overlaid at the
    // exact same position/scale.
    if (hpBadge != NULL) hpBadge->DrawRatio(brush, hpRatio, kBadgeX, kBadgeY, kBadgeScale);
    if (defLine != NULL) defLine->DrawRatio(brush, defRatio, kBadgeX, kBadgeY, kBadgeScale);
    if (atkLine != NULL) atkLine->DrawRatio(brush, atkRatio, kBadgeX, kBadgeY, kBadgeScale);

    if (valueFont != NULL) {
        std::string hpText = "HP  " + std::to_string(hp) + " / " + std::to_string(hpMax);
        std::string defText = "DEF " + std::to_string(def) + " / " + std::to_string(defMax);
        std::string atkText = "ATK " + std::to_string(atk);
        valueFont->Draw(hpText.c_str(), kValueX, kRowHpY, kHpCol, brush);
        valueFont->Draw(defText.c_str(), kValueX, kRowDefY, kDefCol, brush);
        valueFont->Draw(atkText.c_str(), kValueX, kRowAtkY, kAtkCol, brush);
    }
}
