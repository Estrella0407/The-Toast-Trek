#pragma once
#include "GameState.h"
#include <memory>

// The Stardew-style choice screen shown after "Press Enter" on the title:
//   New Game   - wipe the save and start from the forest (+ tutorial)
//   Continue   - resume from savegame.txt (greyed if there is none)
//   Settings   - open the sound settings screen
//   Quit       - close the game
// Up / Down move, Enter selects, Esc goes back to the title.
std::unique_ptr<GameState> CreateMenuSelectState();
