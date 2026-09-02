#include "GameOverState.h"
#include "GameState.h"
#include "OverworldState.h"   // CreateForestState
#include "SoundManage.h"
#include <dinput.h>
#include <cmath>

namespace {
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }
}

GameOverState::GameOverState(SoundManage* soundMgr)
    : titleFont(nullptr)
    , statsFont(nullptr)
    , promptFont(nullptr)
    , retryWasDown(false)
    , menuWasDown(false)
    , soundManage(soundMgr)
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
    // The game-over screen is the only state on the stack
    // Both choices rebuild it with ClearAndPush
    if (JustPressed(context.keys, DIK_R, retryWasDown)) {
        ResetRunProgress(context);   // Fresh Pochi, empty pack, every map locked again
        manager.ClearAndPush(CreateForestState());
    }
    if (JustPressed(context.keys, DIK_M, menuWasDown)) {
        manager.ClearAndPush(CreateMainMenuState());
    }
}

void GameOverState::Update(GameContext& context, GameStateManager& manager) {
    flashTimer += 0.016f;   // ~60 fps

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