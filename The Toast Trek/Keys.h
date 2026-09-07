#pragma once
#include <windows.h>   // BYTE
#include <dinput.h>

// Friendly names for the DirectInput scancodes the game uses, so you can
// write RETURN_KEY instead of remembering DIK_RETURN.
//
//     if (KeyDown(context.keys, RETURN_KEY)) ...          // key held  (folds in & 0x80)
//     if (JustPressed(context.keys, RETURN_KEY, wasDown)) // key just pressed (edge)
//
// Add a line here whenever you need another key.

// Is `key` held down this frame? Wraps the raw "buffer[scancode] & 0x80"
// test (with a null guard) so call sites don't repeat it. Same thing as
// GameScene::IsKeyDown, usable without the GameScene:: prefix.
inline bool KeyDown(const BYTE* keys, int key) {
    return keys != nullptr && (keys[key] & 0x80) != 0;
}

constexpr int RETURN_KEY = DIK_RETURN;
constexpr int ENTER_KEY  = DIK_RETURN;   // same key, alternative name
constexpr int ESCAPE_KEY = DIK_ESCAPE;
constexpr int SPACE_KEY  = DIK_SPACE;

constexpr int UP_KEY    = DIK_UP;
constexpr int DOWN_KEY  = DIK_DOWN;
constexpr int LEFT_KEY  = DIK_LEFT;
constexpr int RIGHT_KEY = DIK_RIGHT;

constexpr int W_KEY = DIK_W;
constexpr int A_KEY = DIK_A;
constexpr int S_KEY = DIK_S;
constexpr int D_KEY = DIK_D;
constexpr int Q_KEY = DIK_Q;
constexpr int E_KEY = DIK_E;
constexpr int F_KEY = DIK_F;
constexpr int K_KEY = DIK_K;
constexpr int L_KEY = DIK_L;
constexpr int M_KEY = DIK_M;
constexpr int R_KEY = DIK_R;

constexpr int NUM1_KEY = DIK_1;
constexpr int NUM2_KEY = DIK_2;
constexpr int NUM3_KEY = DIK_3;
constexpr int NUM4_KEY = DIK_4;

constexpr int F5_KEY  = DIK_F5;
constexpr int F10_KEY = DIK_F10;
