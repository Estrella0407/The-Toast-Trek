#include "GameObject.h"

GameObject::GameObject()
    : position(0.0f, 0.0f), sprite(nullptr)
{
}

GameObject::GameObject(Sprite* ownedSprite)
    : position(0.0f, 0.0f), sprite(ownedSprite)
{
    if (sprite != nullptr) position = sprite->GetPosition();
}

GameObject::~GameObject()
{
    delete sprite;
    sprite = nullptr;
}

void GameObject::Render(LPD3DXSPRITE sharedBrush, D3DCOLOR tint)
{
    if (sprite != nullptr) sprite->Draw(sharedBrush, tint);
}

void GameObject::SetPosition(float x, float y)
{
    position = D3DXVECTOR2(x, y);
    if (sprite != nullptr) sprite->SetPosition(x, y);
}
