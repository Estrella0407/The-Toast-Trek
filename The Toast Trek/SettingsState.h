#pragma once
#include "GameState.h"
#include "Font.h"
#include "SoundManage.h"
#include <memory>

class SettingsState : public GameState {
private:
    Font* titleFont;
    Font* optionFont;
    Font* valueFont;
    Font* controlsFont;
    Font* promptFont;

    int selectedOption;
    int maxOptions;

    bool upWasDown;
    bool downWasDown;
    bool enterWasDown;
    bool escapeWasDown;
    bool leftWasDown;
    bool rightWasDown;

    // Settings values
    float masterVolume;
    float sfxVolume;
    float musicVolume;
    bool fullscreen;
    bool mute;

    // Sound manager reference
    SoundManage* soundManage;

    void UpdateVolumeDisplay();
    void ApplySettings();

public:
    SettingsState(SoundManage* soundMgr);
    ~SettingsState();

    void Initialize(GameContext& context) override;
    void HandleInput(GameContext& context, GameStateManager& manager) override;
    void Update(GameContext& context, GameStateManager& manager) override;
    void Render(GameContext& context) override;
    D3DCOLOR ClearColor() const override;
};

