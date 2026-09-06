#pragma once
#include "GameStateManager.h"
#include "Font.h"
#include "SoundManage.h"
#include <memory>

class GameOverScene : public GameScene {
private:
    Font* titleFont;
    Font* statsFont;
    Font* promptFont;

    bool retryWasDown;
    bool menuWasDown;

    SoundManage* soundManage;

    float flashTimer;   // Drives the title colour flash

public:
    explicit GameOverScene(SoundManage* soundMgr);
    ~GameOverScene();

    void Initialize(GameContext& context) override;
    void HandleInput(GameContext& context, GameStateManager& manager) override;
    void Update(GameContext& context, GameStateManager& manager) override;
    void Render(GameContext& context) override;
    D3DCOLOR ClearColor() const override;
};

// Pushed by BattleScene when Pochi loses a fight
std::unique_ptr<GameScene> CreateGameOverScene(SoundManage* sound);