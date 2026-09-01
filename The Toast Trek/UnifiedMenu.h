#pragma once
#include "GameState.h"
#include <memory>

// The E-key tab menu (Stardew-style). Pushed on top of the overworld, so
// the state stack freezes the game underneath while it's open. `backdrop`
// is the state it was opened over (e.g. the overworld) - the menu redraws
// it behind the dimmed panel so the world still shows through. Pass null
// for an opaque background.
//
// Tabs: Inventory | Status | Settings
//   A / D            switch tabs
//   Up / Down        move the selection within a tab
//   Left / Right     adjust the selected Settings value (else also switch tabs)
//   Enter            use the selected item / toggle mute
//   Mouse            hover to select, click a tab / row / volume bar
//   E or Esc         close
std::unique_ptr<GameState> CreateUnifiedMenuState(GameState* backdrop = nullptr);
