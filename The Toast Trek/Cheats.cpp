#include "Cheats.h"
#include "Keys.h"

namespace Cheats {
    bool enabled = false;

    namespace {
        bool toggleWasDown = false;
    }

    void Update(BYTE* keys) {
        // Compare with last frame for a one-shot press (F5 toggles cheats)
        const bool down = KeyDown(keys, F5_KEY);
        if (down && !toggleWasDown) {
            enabled = !enabled;
        }
        toggleWasDown = down;
    }
}
