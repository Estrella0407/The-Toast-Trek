#include "GameOverState.h"
#include "GameState.h"
#include "OverworldState.h"   // CreateForestState
#include "Pochi.h"
#include "SoundManage.h"
#include <dinput.h>
#include <string>
#include <sstream>
#include <cmath>

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
        soundManage->PlaySfx("gameover", 0.8f);
    }
}

void GameOverState::HandleInput(GameContext& context, GameStateManager& manager) {
    // The game-over screen is the only state on the stack, so both choices
    // rebuild it with ClearAndPush.
    if (JustPressed(context.keys, DIK_R, retryWasDown)) {
        if (context.playerStats != nullptr) context.playerStats->SetLevel(1); // fresh Pochi
        context.clearedMaps.clear();   // restart from scratch - every map locked again
        manager.ClearAndPush(CreateForestState());
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
    LPD3DXSPRITE brush = context.spriteBrush;

    if (titleFont) {
        D3DCOLOR color = (fmod(flashTimer, 0.6f) > 0.3f)
            ? D3DCOLOR_XRGB(200, 50, 50) : D3DCOLOR_XRGB(255, 70, 70);
        titleFont->Draw("GAME OVER", color, brush);
    }
    if (statsFont) {
        statsFont->Draw("Pochi has run out of health.", D3DCOLOR_XRGB(210, 200, 200), brush);
    }
    if (promptFont) {
        promptFont->Draw("R: RETRY FROM THE START     M: MAIN MENU",
            D3DCOLOR_XRGB(200, 200, 200), brush);
    }
}

D3DCOLOR GameOverState::ClearColor() const {
    // Fade to dark red
    return D3DCOLOR_XRGB(35, 10, 10);
}

std::unique_ptr<GameState> CreateGameOverState(SoundManage* sound) {
    return std::make_unique<GameOverState>(sound);
}