#include "Cheats.h"
#include <dinput.h>

namespace Cheats {
    bool enabled = false;

    namespace {
        bool toggleWasDown = false;
    }

    void Update(BYTE* keys) {
        // Lecture 4: a key is held when (keys[DIK_*] & 0x80) != 0; comparing
        // against the previous frame turns that into a single "just pressed".
        const bool down = keys != nullptr && (keys[DIK_F5] & 0x80) != 0;
        if (down && !toggleWasDown) {
            enabled = !enabled;
        }
        toggleWasDown = down;
    }
}
