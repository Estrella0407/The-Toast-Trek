#pragma once
#include "GameState.h"
#include <memory>

// The sound settings screen: Master / Music / SFX volume + Mute
// Reads and writes SoundManage live and persists to settings.txt on close
//   Up / Down     move the selection
//   Left / Right   adjust the value (Enter toggles Mute)
//   Esc / E        close
std::unique_ptr<GameState> CreateSettingsState(GameState* backdrop = nullptr);
