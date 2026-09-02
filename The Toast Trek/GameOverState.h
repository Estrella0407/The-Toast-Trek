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

    // Stats to display
    int score;
    int enemiesDefeated;
    int timeSurvived;
    std::string levelName;

    float animTimer;
    float flashTimer;

public:
    GameOverState(SoundManage* soundMgr, int score = 0, int enemies = 0, int time = 0, const std::string& level = "");
    ~GameOverState();

    void Initialize(GameContext& context) override;
    void HandleInput(GameContext& context, GameStateManager& manager) override;
    void Update(GameContext& context, GameStateManager& manager) override;
    void Render(GameContext& context) override;
    D3DCOLOR ClearColor() const override;
};