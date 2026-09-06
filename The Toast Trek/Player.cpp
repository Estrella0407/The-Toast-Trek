#include "Player.h"

Player::Player(IDirect3DDevice9* device)
    : GameObject(new Sprite(device, "Assets/Characters/Pochi.png", 250, 60, 5, 2, 10, 100.0f, 380.0f))
{
    if (sprite != nullptr) {
        sprite->CropToFrame(0);
        sprite->SetScale(2.0f);
    }
}
