#pragma once
#include "GameState.h"
#include <memory>

// The E-key tab menu
// Pushed on top of the overworld, so the state stack freezes the game underneath while it's open
// `backdrop` is the state it was opened over
//
// Tabs: Inventory | Status | Settings
//	A / D            switch tabs
//  up / Down        move the selection within a tab
//  left / Right     adjust the selected Settings value (else also switch tabs)
//  Enter            use the selected item / toggle mute
//  mouse            hover to select, click a tab / row / volume bar
//  E or Esc         close
std::unique_ptr<GameState> CreateUnifiedMenuState(GameState* backdrop = nullptr);
