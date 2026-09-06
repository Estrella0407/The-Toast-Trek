#pragma once
#include "GameState.h"
#include "Font.h"
#include "SoundManage.h"
#include <memory>

class GameOverState : public GameState {
private:
    Font* titleFont;
    Font* statsFont;
    Font* promptFont;

    bool retryWasDown;
    bool menuWasDown;

    SoundManage* soundManage;

    float flashTimer;   // Drives the title colour flash

public:
    explicit GameOverState(SoundManage* soundMgr);
    ~GameOverState();

    void Initialize(GameContext& context) override;
    void HandleInput(GameContext& context, GameStateManager& manager) override;
    void Update(GameContext& context, GameStateManager& manager) override;
    void Render(GameContext& context) override;
    D3DCOLOR ClearColor() const override;
};