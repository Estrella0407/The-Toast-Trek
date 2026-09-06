#include "GameScene.h"

bool GameScene::IsKeyDown(const BYTE* keys, int dikCode) {
    return keys != nullptr && (keys[dikCode] & 0x80) != 0;
}

bool GameScene::JustPressed(const BYTE* keys, int dikCode, bool& wasDown) {
    const bool down = IsKeyDown(keys, dikCode);
    const bool pressed = down && !wasDown;
    wasDown = down;
    return pressed;
}
