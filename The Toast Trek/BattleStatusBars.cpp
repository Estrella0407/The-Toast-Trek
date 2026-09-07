#include "BattleStatusBars.h"
#include "FrameBar.h"
#include "Font.h"
#include "Pochi.h"
#include <string>

namespace {
    constexpr int kFrames = 8;
    constexpr float kScale = 0.26f;   // on-screen size of the bar art

    // Each redesigned strip has a different native frame size and a
    // different transparent margin around the bar art. artX/artY = top-left
    // of the opaque art inside one frame (measured from the PNGs).
    struct Strip { const char* path; int frameW, frameH, artX, artY, artW, artH; };
    constexpr Strip kHpStrip     { "Assets/UI/hp_strip.png",       1085, 355, 51, 108, 962, 196 };
    constexpr Strip kShieldStrip { "Assets/UI/shield_strip.png",   1085, 369, 49,  52, 964, 199 };
    constexpr Strip kEnemyStrip  { "Assets/UI/enemy_hp_strip.png", 1024, 256, 34,  31, 964, 181 };

    // On-screen top-left of the VISIBLE bar art.
    constexpr float kHpArtX = 14.0f, kHpArtY = 10.0f;
    constexpr float kShieldArtX = 14.0f, kShieldArtY = 74.0f;
    constexpr float kEnemyArtX = 1280.0f - kEnemyStrip.artW * kScale - 16.0f;
    constexpr float kEnemyArtY = 10.0f;

    // "x / y" readouts.
    constexpr float kPochiTextX = kHpArtX + kHpStrip.artW * kScale + 12.0f;
    constexpr float kHpTextY = kHpArtY + kHpStrip.artH * kScale * 0.5f - 8.0f;
    constexpr float kDefTextY = kShieldArtY + kShieldStrip.artH * kScale * 0.5f - 8.0f;
    constexpr float kEnemyTextRight = kEnemyArtX + kEnemyStrip.artW * kScale;
    constexpr float kEnemyTextY = kEnemyArtY + kEnemyStrip.artH * kScale + 4.0f;
    constexpr float kGlyphW = 8.5f;   // ~px per char at font size 15

    const D3DCOLOR kHpCol = D3DCOLOR_XRGB(226, 92, 66);
    const D3DCOLOR kDefCol = D3DCOLOR_XRGB(96, 176, 230);
    const D3DCOLOR kEnemyCol = D3DCOLOR_XRGB(226, 92, 66);
}

BattleStatusBars::BattleStatusBars(IDirect3DDevice9* device)
    : healthBar(NULL), shieldBar(NULL), enemyBar(NULL), valueFont(NULL) {
    healthBar = new FrameBar(device, kHpStrip.path, kHpStrip.frameW, kHpStrip.frameH, kFrames);
    shieldBar = new FrameBar(device, kShieldStrip.path, kShieldStrip.frameW, kShieldStrip.frameH, kFrames);
    enemyBar = new FrameBar(device, kEnemyStrip.path, kEnemyStrip.frameW, kEnemyStrip.frameH, kFrames);
    valueFont = new Font(device, 0.0f, 0.0f, 120, 20, 15, "Arial");
}

BattleStatusBars::~BattleStatusBars() {
    delete healthBar;
    delete shieldBar;
    delete enemyBar;
    delete valueFont;
}

namespace {
    // Draw a strip so its VISIBLE art top-left lands at (artX, artY).
    void DrawStrip(FrameBar* bar, LPD3DXSPRITE brush, const Strip& s,
                   float ratio, float artX, float artY) {
        if (bar == NULL) return;
        bar->DrawRatio(brush, ratio, artX - s.artX * kScale, artY - s.artY * kScale, kScale);
    }
}

void BattleStatusBars::Draw(LPD3DXSPRITE brush, const Pochi& stats,
                            float enemyHpRatio, int enemyHp, int enemyMaxHp) {
    const int hp = stats.GetHealth();
    const int hpMax = stats.GetMaxHealth();
    const int def = stats.GetArmor();
    const int defMax = stats.GetMaxArmor();
    const float hpRatio = hpMax > 0 ? (float)hp / hpMax : 0.0f;
    const float defRatio = defMax > 0 ? (float)def / defMax : 0.0f;

    DrawStrip(healthBar, brush, kHpStrip, hpRatio, kHpArtX, kHpArtY);
    DrawStrip(shieldBar, brush, kShieldStrip, defRatio, kShieldArtX, kShieldArtY);
    DrawStrip(enemyBar, brush, kEnemyStrip, enemyHpRatio, kEnemyArtX, kEnemyArtY);

    if (valueFont != NULL) {
        const std::string hpText = std::to_string(hp) + " / " + std::to_string(hpMax);
        const std::string defText = std::to_string(def) + " / " + std::to_string(defMax);
        const std::string enemyText = std::to_string(enemyHp < 0 ? 0 : enemyHp)
                                    + " / " + std::to_string(enemyMaxHp);
        valueFont->Draw(hpText.c_str(), kPochiTextX, kHpTextY, kHpCol, brush);
        valueFont->Draw(defText.c_str(), kPochiTextX, kDefTextY, kDefCol, brush);
        const float enemyTextX = kEnemyTextRight - (float)enemyText.size() * kGlyphW;
        valueFont->Draw(enemyText.c_str(), enemyTextX, kEnemyTextY, kEnemyCol, brush);
    }
}
