#pragma once
#include "GameState.h"
#include <memory>

// The state stack only ticks and renders its top entry
// The forest underneath freezes for free while this is up
// Controls: A / Left and D / Right page through
// Enter or Space advances, and closes the popup on the last page
std::unique_ptr<GameState> CreateForestIntroPopup();
