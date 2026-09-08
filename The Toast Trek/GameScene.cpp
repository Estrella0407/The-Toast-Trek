#include "GameScene.h"
#include "Keys.h"

bool GameScene::JustPressed(const BYTE* keys, int key, bool& wasDown) {
    const bool down = KeyDown(keys, key);
    const bool pressed = down && !wasDown;
    wasDown = down;
    return pressed;
}
