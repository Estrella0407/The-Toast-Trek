#pragma once
#include "GameState.h"
#include <memory>

// The ending, pushed once the Maki fight in the ruins interior is won
//   1. Walk-in  - Denji walks up the entrance corridor to Pochi
//   2. Dialogue - A two-line bottom panel (F / Enter advances)
//   3. Credits  - A black screen with the credit lines scrolling up and a
//                 ball bouncing around the window. Enter / Esc quits to menu
std::unique_ptr<GameState> CreateEndingState();
