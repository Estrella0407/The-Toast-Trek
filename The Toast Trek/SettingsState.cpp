//#include "SettingsState.h"
//#include "GameState.h"
//#include <dinput.h>
//#include <string>
//
//namespace {
//    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
//        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
//        bool pressed = isDown && !wasDown;
//        wasDown = isDown;
//        return pressed;
//    }
//}
//
//SettingsState::SettingsState(SoundManage* soundMgr)
//    : titleFont(nullptr)
//    , optionFont(nullptr)
//    , valueFont(nullptr)
//    , controlsFont(nullptr)
//    , promptFont(nullptr)
//    , selectedOption(0)
//    , maxOptions(5) // Master, SFX, Music, Fullscreen, Mute
//    , upWasDown(false)
//    , downWasDown(false)
//    , enterWasDown(false)
//    , escapeWasDown(false)
//    , leftWasDown(false)
//    , rightWasDown(false)
//    , masterVolume(1.0f)
//    , sfxVolume(1.0f)
//    , musicVolume(0.8f)
//    , fullscreen(false)
//    , mute(false)
//    , soundManage(soundMgr) {
//}
//
//SettingsState::~SettingsState() {
//    delete titleFont;
//    delete optionFont;
//    delete valueFont;
//    delete controlsFont;
//    delete promptFont;
//}
//
//void SettingsState::Initialize(GameContext& context) {
//    titleFont = new Font(context.device, 0.0f, 80.0f, 1280, 70, 48, "Arial");
//    optionFont = new Font(context.device, 200.0f, 200.0f, 500, 40, 28, "Arial");
//    valueFont = new Font(context.device, 800.0f, 200.0f, 300, 40, 28, "Arial");
//    controlsFont = new Font(context.device, 0.0f, 600.0f, 1280, 30, 20, "Arial");
//    promptFont = new Font(context.device, 0.0f, 650.0f, 1280, 30, 20, "Arial");
//
//    // Load current settings from sound manager
//    if (soundManage) {
//        masterVolume = soundManage->GetMasterVolume();
//        sfxVolume = soundManage->GetSFXVolume();
//        musicVolume = soundManage->GetMusicVolume();
//        mute = soundManage->IsMuted();
//    }
//}
//
//void SettingsState::UpdateVolumeDisplay() {
//    // This is called when volume changes - can be used for visual feedback
//}
//
//void SettingsState::ApplySettings() {
//    if (soundManage) {
//        soundManage->SetMasterVolume(masterVolume);
//        soundManage->SetSFXVolume(sfxVolume);
//        soundManage->SetMusicVolume(musicVolume);
//        soundManage->SetMute(mute);
//    }
//}
//
//void SettingsState::HandleInput(GameContext& context, GameStateManager& manager) {
//    BYTE* keys = context.keys;
//
//    if (JustPressed(keys, DIK_ESCAPE, escapeWasDown)) {
//        ApplySettings();
//        manager.Pop();
//        return;
//    }
//
//    if (JustPressed(keys, DIK_UP, upWasDown)) {
//        selectedOption = (selectedOption - 1 + maxOptions) % maxOptions;
//    }
//
//    if (JustPressed(keys, DIK_DOWN, downWasDown)) {
//        selectedOption = (selectedOption + 1) % maxOptions;
//    }
//
//    if (JustPressed(keys, DIK_LEFT, leftWasDown)) {
//        switch (selectedOption) {
//        case 0: // Master Volume
//            masterVolume = max(0.0f, masterVolume - 0.1f);
//            break;
//        case 1: // SFX Volume
//            sfxVolume = max(0.0f, sfxVolume - 0.1f);
//            break;
//        case 2: // Music Volume
//            musicVolume = max(0.0f, musicVolume - 0.1f);
//            break;
//        case 3: // Fullscreen
//            fullscreen = !fullscreen;
//            // TODO: Implement fullscreen toggle
//            break;
//        case 4: // Mute
//            mute = !mute;
//            break;
//        }
//        ApplySettings();
//    }
//
//    if (JustPressed(keys, DIK_RIGHT, rightWasDown)) {
//        switch (selectedOption) {
//        case 0: // Master Volume
//            masterVolume = min(1.0f, masterVolume + 0.1f);
//            break;
//        case 1: // SFX Volume
//            sfxVolume = min(1.0f, sfxVolume + 0.1f);
//            break;
//        case 2: // Music Volume
//            musicVolume = min(1.0f, musicVolume + 0.1f);
//            break;
//        case 3: // Fullscreen
//            fullscreen = !fullscreen;
//            // TODO: Implement fullscreen toggle
//            break;
//        case 4: // Mute
//            mute = !mute;
//            break;
//        }
//        ApplySettings();
//    }
//
//    if (JustPressed(keys, DIK_RETURN, enterWasDown)) {
//        if (selectedOption == 3) {
//            fullscreen = !fullscreen;
//            // TODO: Implement fullscreen toggle
//        }
//        else if (selectedOption == 4) {
//            mute = !mute;
//            ApplySettings();
//        }
//    }
//}
//
//void SettingsState::Update(GameContext& context, GameStateManager& manager) {
//    // Update sound system
//    if (soundManage) {
//        soundManage->Update();
//    }
//}
//
//void SettingsState::Render(GameContext& context) {
//    const char* options[] = {
//        "MASTER VOLUME",
//        "SFX VOLUME",
//        "MUSIC VOLUME",
//        "FULLSCREEN",
//        "MUTE"
//    };
//
//    char valueBuffer[64];
//
//    if (titleFont) {
//        titleFont->Draw("SETTINGS", D3DCOLOR_XRGB(255, 255, 255));
//    }
//
//    float yPos = 200.0f;
//    for (int i = 0; i < maxOptions; i++) {
//        D3DCOLOR color = (i == selectedOption) ? D3DCOLOR_XRGB(255, 255, 100) : D3DCOLOR_XRGB(200, 200, 200);
//
//        if (optionFont) {
//            optionFont->Draw(options[i], 200.0f, yPos, color);
//        }
//
//        if (valueFont) {
//            std::string value;
//            switch (i) {
//            case 0: // Master Volume
//                sprintf_s(valueBuffer, "%d%%", (int)(masterVolume * 100));
//                value = valueBuffer;
//                break;
//            case 1: // SFX Volume
//                sprintf_s(valueBuffer, "%d%%", (int)(sfxVolume * 100));
//                value = valueBuffer;
//                break;
//            case 2: // Music Volume
//                sprintf_s(valueBuffer, "%d%%", (int)(musicVolume * 100));
//                value = valueBuffer;
//                break;
//            case 3: // Fullscreen
//                value = fullscreen ? "ON" : "OFF";
//                break;
//            case 4: // Mute
//                value = mute ? "ON" : "OFF";
//                break;
//            }
//            valueFont->Draw(value.c_str(), 800.0f, yPos, color);
//        }
//        yPos += 50.0f;
//    }
//
//    if (controlsFont) {
//        controlsFont->Draw("ARROW KEYS: Navigate  |  ENTER: Toggle  |  ESC: Back",
//            D3DCOLOR_XRGB(150, 150, 150));
//    }
//
//    if (promptFont) {
//        promptFont->Draw("PRESS ESC TO RETURN", D3DCOLOR_XRGB(100, 100, 100));
//    }
//}
//
//D3DCOLOR SettingsState::ClearColor() const {
//    return D3DCOLOR_XRGB(20, 30, 50);
//}
//
