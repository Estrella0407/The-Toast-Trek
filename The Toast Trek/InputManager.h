#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "DirectInput.h"
#include "Keys.h"

// Game-facing input: owns the DirectInput device pair for the keyboard
// buffer, and tracks an absolute cursor position (mapped into back-buffer
// space) plus the left-button state, which the DirectInput mouse - configured
// for relative deltas here - can't give directly.
class InputManager {
private:
    DirectInput directInput;

    float mouseX;
    float mouseY;
    bool mouseLeftDown;

public:
    InputManager();

    bool Create(HWND hWnd);

    // Poll the devices and refresh the cursor mapping for this frame.
    void Update(HWND hWnd, int backBufferWidth, int backBufferHeight);

    const BYTE* Keys() const { return directInput.Keys(); }
    bool IsKeyDown(int key) const { return KeyDown(directInput.Keys(), key); }

    float MouseX() const { return mouseX; }
    float MouseY() const { return mouseY; }
    bool MouseLeftDown() const { return mouseLeftDown; }

    void Cleanup() { directInput.Cleanup(); }
};
