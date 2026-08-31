#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Font;
class Pochi;
class FrameBar;

// Battle HUD: framed health + shield bars (top-left) and the enemy's HP
// bar (under the enemy), every fill level a pre-drawn frame in a vertical
// strip PNG. Create one per battle; call Draw() in Render().
class BattleStatusBars {
public:
    explicit BattleStatusBars(IDirect3DDevice9* device);
    ~BattleStatusBars();

    // enemyHpRatio in [0, 1].
    void Draw(LPD3DXSPRITE brush, const Pochi& stats, float enemyHpRatio);

private:
    FrameBar* healthBar;   // Assets/UI/health_strip.png    - frame = health
    FrameBar* shieldBar;   // Assets/UI/shield_strip.png    - frame = armor
    FrameBar* enemyBar;    // Assets/UI/enemy_hp_strip.png  - frame = ratio bucket
    Font* valueFont;
};
