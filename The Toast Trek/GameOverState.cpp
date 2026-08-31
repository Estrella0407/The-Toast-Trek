#include "GameOverState.h"
#include "GameState.h"
#include <dinput.h>
#include <string>
#include <sstream>

namespace {
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }
}

GameOverState::GameOverState(SoundManage* soundMgr, int score, int enemies, int time, const std::string& level)
    : titleFont(nullptr)
    , statsFont(nullptr)
    , promptFont(nullptr)
    , retryWasDown(false)
    , menuWasDown(false)
    , soundManage(soundMgr)
    , score(score)
    , enemiesDefeated(enemies)
    , timeSurvived(time)
    , levelName(level)
    , animTimer(0.0f)
    , flashTimer(0.0f) {
}

GameOverState::~GameOverState() {
    delete titleFont;
    delete statsFont;
    delete promptFont;
}

void GameOverState::Initialize(GameContext& context) {
    titleFont = new Font(context.device, 0.0f, 180.0f, 1280, 80, 56, "Arial");
    statsFont = new Font(context.device, 0.0f, 300.0f, 1280, 50, 26, "Arial");
    promptFont = new Font(context.device, 0.0f, 500.0f, 1280, 60, 22, "Arial");

    if (soundManage) {
        // Play game over sound
        soundManage->PlaySound("gameover", 0.8f);
    }
}

void GameOverState::HandleInput(GameContext& context, GameStateManager& manager) {
    if (JustPressed(context.keys, DIK_R, retryWasDown)) {
        manager.Pop(); // Retry the level
    }
    if (JustPressed(context.keys, DIK_M, menuWasDown)) {
        manager.ClearAndPush(CreateMainMenuState());
    }
}

void GameOverState::Update(GameContext& context, GameStateManager& manager) {
    animTimer += 0.016f; // ~60fps
    flashTimer += 0.016f;

    if (soundManage) {
        soundManage->Update();
    }
}

void GameOverState::Render(GameContext& context) {
    if (titleFont) {
        // Flash effect for title
        D3DCOLOR color = D3DCOLOR_XRGB(255, 0, 0);
        if (fmod(flashTimer, 0.5f) > 0.25f) {
            color = D3DCOLOR_XRGB(200, 50, 50);
        }
        titleFont->Draw("GAME OVER", color);
    }

    if (statsFont) {
        std::stringstream stats;
        stats << "LEVEL: " << levelName << "\n";
        stats << "SCORE: " << score << "\n";
        stats << "ENEMIES DEFEATED: " << enemiesDefeated << "\n";
        stats << "TIME SURVIVED: " << timeSurvived << "s";

        statsFont->Draw(stats.str().c_str(), 420.0f, 280.0f, D3DCOLOR_XRGB(220, 220, 220));
    }

    if (promptFont) {
        promptFont->Draw("R: RETRY    M: RETURN TO MAIN MENU",
            D3DCOLOR_XRGB(200, 200, 200));
    }
}

D3DCOLOR GameOverState::ClearColor() const {
    // Fade to dark red
    return D3DCOLOR_XRGB(35, 10, 10);
}