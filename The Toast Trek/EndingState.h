#pragma once
#include "GameState.h"
#include <memory>

// The ending, pushed once the Maki fight in the ruins interior is won.
// Three phases:
//   1. Walk-in  - the ruins-interior map, Pochi where the fight ended,
//                 Denji walking up from the entrance corridor.
//   2. Dialogue - a bottom panel: "Pochi... I finally found you!" ->
//                 "Let's get you home." (F / Enter advances).
//   3. Credits  - cuts to a black credit roll: Pochi on the left, Denji on
//                 the right, the credit lines auto-scrolling up the middle,
//                 and a round ball knocked between the two. The ball moves
//                 under Newtonian velocity + gravity + friction (Lecture 6),
//                 bounces off the arena box with a "deflect" response
//                 (Lecture 6), is rallied back on circular contact with a
//                 character (Lecture 6), and auto-serves itself when it goes
//                 idle so the scene plays on its own. F kicks it manually.
//                 Timing is FrameTimer (Lecture 5), input DirectInput
//                 (Lecture 4). Enter / Esc returns to the main menu.
std::unique_ptr<GameState> CreateEndingState();
