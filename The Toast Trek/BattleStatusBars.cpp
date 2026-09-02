#include "BattleStatusBars.h"
#include "Font.h"
#include "Pochi.h"
#include "UiFill.h"
#include <algorithm>
#include <string>

namespace {
    // --- Exact pixel size of each art file ------------------------------
    constexpr int kHealthW = 1028, kHealthH = 340;
    constexpr int kShieldW = 1025, kShieldH = 341;
    constexpr int kEnemyW = 2172, kEnemyH = 724;

    // Where the coloured fill sits inside each bar PNG, as 0..1 fractions (x l..r, y t..b)
    struct FillBox { float l, r, t, b; };
    constexpr FillBox kFill{ 0.16f, 0.94f, 0.30f, 0.70f };

    const D3DCOLOR kEmptyWash = D3DCOLOR_ARGB(255, 58, 36, 24);   // Matches PochiBadge

    // --- On-screen placement -------------------------------------------
    constexpr float kBarScale = 0.24f;                       // health + shield
    constexpr float kHealthX = 12.0f, kHealthY = 6.0f;
    constexpr float kShieldX = 12.0f;
    constexpr float kShieldY = kHealthY + kHealthH * kBarScale * 0.62f;   // Just below health

    // Enemy bar: same on-screen width as Pochi's bars, pinned top-right
    constexpr float kEnemyScale = kHealthW * kBarScale / (float)kEnemyW;
    constexpr float kEnemyX = 1280.0f - kEnemyW * kEnemyScale - 12.0f;
    constexpr float kEnemyY = 6.0f;

    // Value readout beside Pochi's bars
    constexpr float kValueX = kHealthX + kHealthW * kBarScale + 14.0f;
    constexpr float kHpTextY = kHealthY + 20.0f;
    constexpr float kDefTextY = kShieldY + 20.0f;

    // Matched to PochiBadge so both HUDs share one palette
    const D3DCOLOR kHpCol = D3DCOLOR_XRGB(255, 138, 118);
    const D3DCOLOR kDefCol = D3DCOLOR_XRGB(150, 205, 255);

    // Wash the drained tail of a bar drawn at (x, y), art size artW x artH,
    // on-screen scale s, filled to `ratio`
    void DrainBar(LPD3DXSPRITE brush, IDirect3DTexture9* white,
                  float x, float y, int artW, int artH, float s, float ratio) {
        if (white == NULL) return;
        ratio = std::clamp(ratio, 0.0f, 1.0f);
        const float fillL = kFill.l * artW;
        const float fillR = kFill.r * artW;
        const float xFill = fillL + ratio * (fillR - fillL);   // Right edge of the fill
        if (xFill >= fillR) return;
        ui::FillRect(brush, white,
            x + xFill * s, y + kFill.t * artH * s,
            (fillR - xFill) * s, (kFill.b - kFill.t) * artH * s, kEmptyWash);
    }
}

BattleStatusBars::BattleStatusBars(IDirect3DDevice9* device)
    : healthTex(NULL), shieldTex(NULL), enemyTex(NULL), whiteTex(NULL), valueFont(NULL) {
    healthTex = ui::LoadTexture(device, "Assets/UI/health_bar_full.png", kHealthW, kHealthH);
    shieldTex = ui::LoadTexture(device, "Assets/UI/shield_bar_full.png", kShieldW, kShieldH);
    enemyTex = ui::LoadTexture(device, "Assets/UI/enemy_hp.png", kEnemyW, kEnemyH);
    whiteTex = ui::MakeWhiteTexture(device);
    valueFont = new Font(device, 0.0f, 0.0f, 90, 20, 15, "Arial");
}

BattleStatusBars::~BattleStatusBars() {
    if (healthTex != NULL) healthTex->Release();
    if (shieldTex != NULL) shieldTex->Release();
    if (enemyTex != NULL) enemyTex->Release();
    if (whiteTex != NULL) whiteTex->Release();
    delete valueFont;
}

void BattleStatusBars::Draw(LPD3DXSPRITE brush, const Pochi& stats, float enemyHpRatio) {
    const int hp = stats.GetHealth();
    const int hpMax = stats.GetMaxHealth();
    const int def = stats.GetArmor();
    const int defMax = stats.GetMaxArmor();
    const float hpRatio = hpMax > 0 ? (float)hp / hpMax : 0.0f;
    const float defRatio = defMax > 0 ? (float)def / defMax : 0.0f;

    // Each bar: draw the full art, then wash the drained tail
    if (healthTex != NULL) {
        ui::DrawTexture(brush, healthTex, kHealthW, kHealthH, kHealthX, kHealthY, kBarScale, kBarScale);
        DrainBar(brush, whiteTex, kHealthX, kHealthY, kHealthW, kHealthH, kBarScale, hpRatio);
    }
    if (shieldTex != NULL) {
        ui::DrawTexture(brush, shieldTex, kShieldW, kShieldH, kShieldX, kShieldY, kBarScale, kBarScale);
        DrainBar(brush, whiteTex, kShieldX, kShieldY, kShieldW, kShieldH, kBarScale, defRatio);
    }
    if (enemyTex != NULL) {
        ui::DrawTexture(brush, enemyTex, kEnemyW, kEnemyH, kEnemyX, kEnemyY, kEnemyScale, kEnemyScale);
        DrainBar(brush, whiteTex, kEnemyX, kEnemyY, kEnemyW, kEnemyH, kEnemyScale, enemyHpRatio);
    }

    if (valueFont != NULL) {
        const std::string hpText = std::to_string(hp) + " / " + std::to_string(hpMax);
        const std::string defText = std::to_string(def) + " / " + std::to_string(defMax);
        valueFont->Draw(hpText.c_str(), kValueX + 1.0f, kHpTextY + 1.0f, ui::kShadow, brush);
        valueFont->Draw(hpText.c_str(), kValueX, kHpTextY, kHpCol, brush);
        valueFont->Draw(defText.c_str(), kValueX + 1.0f, kDefTextY + 1.0f, ui::kShadow, brush);
        valueFont->Draw(defText.c_str(), kValueX, kDefTextY, kDefCol, brush);
    }
}
