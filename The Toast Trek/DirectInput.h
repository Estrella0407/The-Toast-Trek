#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <dinput.h>

// Thin wrapper over a DirectInput8 keyboard + mouse device pair. Poll()
// refreshes the immediate state each frame and re-acquires on focus loss.
class DirectInput {
private:
    LPDIRECTINPUT8 dInput;
    LPDIRECTINPUTDEVICE8 keyboardDevice;
    LPDIRECTINPUTDEVICE8 mouseDevice;

    BYTE keys[256];
    DIMOUSESTATE mouseState;

public:
    DirectInput();

    bool Create(HWND hWnd);
    void Poll();

    const BYTE* Keys() const { return keys; }
    const DIMOUSESTATE& Mouse() const { return mouseState; }

    void Cleanup();
};
