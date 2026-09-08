#pragma once
#include <windows.h>   // BYTE, GetAsyncKeyState
#include <dinput.h>    // DIK_* scancodes

// ---------------------------------------------------------------------------
// DirectInput keyboard
// ---------------------------------------------------------------------------
constexpr int RETURN_KEY = DIK_RETURN;   // 0x1C
constexpr int ENTER_KEY  = DIK_RETURN;   // 0x1C
constexpr int ESCAPE_KEY = DIK_ESCAPE;   // 0x01
constexpr int SPACE_KEY  = DIK_SPACE;    // 0x39

constexpr int UP_KEY    = DIK_UP;        // 0xC8
constexpr int DOWN_KEY  = DIK_DOWN;      // 0xD0
constexpr int LEFT_KEY  = DIK_LEFT;      // 0xCB
constexpr int RIGHT_KEY = DIK_RIGHT;     // 0xCD

constexpr int W_KEY = DIK_W;             // 0x11
constexpr int A_KEY = DIK_A;             // 0x1E
constexpr int S_KEY = DIK_S;             // 0x1F
constexpr int D_KEY = DIK_D;             // 0x20
constexpr int Q_KEY = DIK_Q;             // 0x10
constexpr int E_KEY = DIK_E;             // 0x12
constexpr int F_KEY = DIK_F;             // 0x21
constexpr int K_KEY = DIK_K;             // 0x25
constexpr int L_KEY = DIK_L;             // 0x26
constexpr int M_KEY = DIK_M;             // 0x32
constexpr int R_KEY = DIK_R;             // 0x13

constexpr int NUM1_KEY = DIK_1;          // 0x02
constexpr int NUM2_KEY = DIK_2;          // 0x03
constexpr int NUM3_KEY = DIK_3;          // 0x04
constexpr int NUM4_KEY = DIK_4;          // 0x05

constexpr int F5_KEY  = DIK_F5;          // 0x3F
constexpr int F10_KEY = DIK_F10;         // 0x44

inline bool KeyDown(const BYTE* keys, int key) {
    return keys != nullptr && (keys[key] & 0x80) != 0;
}

inline bool KeyHeldAsync(int vkey) {
    return (GetAsyncKeyState(vkey) & 0x8000) != 0;
}
