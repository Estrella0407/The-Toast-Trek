#include "Cheats.h"
#include <dinput.h>

namespace Cheats {
    bool enabled = false;

    namespace {
        bool toggleWasDown = false;
    }

    void Update(BYTE* keys) {
        // 0x80 bit = key held; compare with last frame for a one-shot press
        const bool down = keys != nullptr && (keys[DIK_F5] & 0x80) != 0;
        if (down && !toggleWasDown) {
            enabled = !enabled;
        }
        toggleWasDown = down;
    }
}
