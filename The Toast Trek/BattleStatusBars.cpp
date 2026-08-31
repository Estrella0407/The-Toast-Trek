#include "BattleStatusBars.h"
#include "FrameBar.h"
#include "Font.h"
#include "Pochi.h"
#include <string>

namespace {
    // --- Frame strip layout (must match the PNGs) -----------------------
    // Frame chosen by ratio, so a full bar always reads as full whatever
    // the level's max is.
    // Power-of-two strips, all the same shape: 1024x2048, 8 frames of
    // 1024x256 (the bar art is letterboxed inside each frame).
    constexpr int kBarW = 1024, kBarH = 256, kBarFrames = 8;
    constexpr int kHealthW = kBarW, kHealthH = kBarH, kHealthFrames = kBarFrames;
    constexpr int kShieldW = kBarW, kShieldH = kBarH, kShieldFrames = kBarFrames;
    constexpr int kEnemyW = kBarW, kEnemyH = kBarH, kEnemyFrames = kBarFrames;

    // --- On-screen placement. Tweak freely. --------------------------
    // Same scale for all three so they sit at the same height. Each frame
    // is 1024x256 art letterboxed to ~72% height, so stacking the health
    // and shield frames ~44px apart leaves a clean gap between the bars.
    constexpr float kBarScale = 0.235f;
    constexpr float kHealthX = 12.0f, kHealthY = 0.0f, kHealthScale = kBarScale;
    // The art is letterboxed ~24px inside each 256-tall frame, so ~11px of
    // that is transparent at this scale. Offset the shield frame past the
    // health frame's real content plus a visible gap.
    constexpr float kShieldX = 12.0f, kShieldY = 58.0f, kShieldScale = kBarScale;
    // Enemy HP bar: top-right corner (its own evil-heart art, no tint needed).
    constexpr float kEnemyScale = kBarScale;
    constexpr float kEnemyX = 1280.0f - kEnemyW * kEnemyScale - 10.0f;
    constexpr float kEnemyY = 0.0f;
    const D3DCOLOR kEnemyTint = D3DCOLOR_ARGB(255, 255, 255, 255);

    // Value text, roughly centred on each bar, to the right of it.
    constexpr float kHpTextX = 258.0f, kHpTextY = 22.0f;
    constexpr float kDefTextX = 258.0f, kDefTextY = 88.0f;

    const D3DCOLOR kHpCol = D3DCOLOR_XRGB(226, 92, 66);
    const D3DCOLOR kDefCol = D3DCOLOR_XRGB(96, 176, 230);
}

BattleStatusBars::BattleStatusBars(IDirect3DDevice9* device)
    : healthBar(NULL), shieldBar(NULL), enemyBar(NULL), valueFont(NULL) {
    healthBar = new FrameBar(device, "Assets/UI/health_strip.png",
                             kHealthW, kHealthH, kHealthFrames);
    shieldBar = new FrameBar(device, "Assets/UI/shield_strip.png",
                             kShieldW, kShieldH, kShieldFrames);
    enemyBar = new FrameBar(device, "Assets/UI/enemy_hp_strip.png",
                            kEnemyW, kEnemyH, kEnemyFrames);
    valueFont = new Font(device, 0.0f, 0.0f, 90, 20, 15, "Arial");
}

BattleStatusBars::~BattleStatusBars() {
    delete healthBar;
    delete shieldBar;
    delete enemyBar;
    delete valueFont;
}

void BattleStatusBars::Draw(LPD3DXSPRITE brush, const Pochi& stats, float enemyHpRatio) {
    const int hp = stats.GetHealth();
    const int hpMax = stats.GetMaxHealth();
    const int def = stats.GetArmor();
    const int defMax = stats.GetMaxArmor();
    const float hpRatio = hpMax > 0 ? (float)hp / hpMax : 0.0f;
    const float defRatio = defMax > 0 ? (float)def / defMax : 0.0f;

    if (healthBar != NULL) healthBar->DrawRatio(brush, hpRatio, kHealthX, kHealthY, kHealthScale);
    if (shieldBar != NULL) shieldBar->DrawRatio(brush, defRatio, kShieldX, kShieldY, kShieldScale);
    if (enemyBar != NULL) enemyBar->DrawRatio(brush, enemyHpRatio, kEnemyX, kEnemyY, kEnemyScale, kEnemyTint);

    if (valueFont != NULL) {
        std::string hpText = std::to_string(hp) + " / " + std::to_string(hpMax);
        std::string defText = std::to_string(def) + " / " + std::to_string(defMax);
        valueFont->Draw(hpText.c_str(), kHpTextX, kHpTextY, kHpCol, brush);
        valueFont->Draw(defText.c_str(), kDefTextX, kDefTextY, kDefCol, brush);
    }
}
