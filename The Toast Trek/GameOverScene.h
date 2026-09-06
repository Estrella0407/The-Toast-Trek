#pragma once
#include "GameStateManager.h"
#include "Font.h"
#include "SoundManager.h"
#include <memory>

class GameOverScene : public GameScene {
private:
    Font* titleFont;
    Font* statsFont;
    Font* promptFont;

    bool retryWasDown;
    bool menuWasDown;

    SoundManager* soundManage;

    float flashTimer;   // Drives the title colour flash

public:
    explicit GameOverScene(SoundManager* soundMgr);
    ~GameOverScene();

    void Initialize(GameContext& context) override;
    void HandleInput(GameContext& context, GameStateManager& manager) override;
    void Update(GameContext& context, GameStateManager& manager) override;
    void Render(GameContext& context) override;
    D3DCOLOR ClearColor() const override;
};

// Pushed by BattleScene when Pochi loses a fight
std::unique_ptr<GameScene> CreateGameOverScene(SoundManager* sound);