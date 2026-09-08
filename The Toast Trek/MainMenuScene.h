#pragma once
#include <memory>
#include "GameStateManager.h"

// The title screen: animated Pochi + "Press Enter". Enter opens the
// Stardew-style choice menu (New Game / Continue / Settings / Quit).
std::unique_ptr<GameScene> CreateMainMenuScene();
