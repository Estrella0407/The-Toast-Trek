#pragma once
#include <Windows.h>

// Developer cheat switch. F5 toggles it; while on:
//  overworld - 3x walk speed, no wall clipping, K clears the nearest boss, L jumps to the map's exit
//  battle    - K wins instantly, Pochi kept at full health/armor
// A red "CHEAT MODE" label shows while it is on
namespace Cheats {
    extern bool enabled;

    // Reads F5 from the key buffer and flips `enabled` on a fresh press
    void Update(BYTE* keys);
}
