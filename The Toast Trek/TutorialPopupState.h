#pragma once
#include "GameState.h"
#include <memory>

// A modal, paged intro (Cult-of-the-Lamb style) shown once, on top of the
// forest. The state stack only ticks and renders its top entry, so the
// forest underneath freezes for free while this is up; this state redraws a
// static snapshot of the forest behind a dimmed panel so the popup still
// reads as sitting "over" the game.
//
// Controls: A / Left and D / Right page through; Enter or Space advances,
// and closes the popup on the last page.
std::unique_ptr<GameState> CreateForestIntroPopup();
