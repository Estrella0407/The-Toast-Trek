#pragma once
#include <Windows.h>

// A tiny developer cheat switch, wired through the same DirectInput key
// buffer the rest of the game reads (Lecture 4). Update() is called once per
// frame from the main loop; F5 toggles the whole thing on/off.
//
// While enabled:
//   * Overworld  - 3x walk speed, walk through walls, K clears the nearest
//                  boss without a fight, L jumps to the map's exit.
//   * Battle     - K wins the fight instantly, and Pochi is kept at full
//                  health/armor so a test run can't be lost.
// (Letter keys rather than F8/F9: on many laptops the Fn row toggles
// airplane mode / brightness and the game never receives the keypress.)
// A red "CHEAT MODE" label is drawn on screen whenever it is on.
namespace Cheats {
    extern bool enabled;

    // Edge-detects F5 on the DirectInput buffer and flips `enabled`.
    void Update(BYTE* keys);
}
