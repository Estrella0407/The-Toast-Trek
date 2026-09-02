#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Font;
class Pochi;

// Battle HUD: Pochi's health + shield bars  and the enemy's HP bar
// Create one per battle; call Draw() in Render()
class BattleStatusBars {
public:
    explicit BattleStatusBars(IDirect3DDevice9* device);
    ~BattleStatusBars();

    // EnemyHpRatio in [0, 1]
    void Draw(LPD3DXSPRITE brush, const Pochi& stats, float enemyHpRatio);

private:
    IDirect3DTexture9* healthTex;   // Assets/UI/health_bar_full.png
    IDirect3DTexture9* shieldTex;   // Assets/UI/shield_bar_full.png
    IDirect3DTexture9* enemyTex;    // Assets/UI/enemy_hp.png
    IDirect3DTexture9* whiteTex;    // 1x1 white - drained-tail wash + text shadow
    Font* valueFont;
};
