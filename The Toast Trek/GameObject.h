#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"

// Base for anything that lives in the world with a position and an owned
// sprite: the player, items, enemies, hearts, projectiles, demo balls.
// Gives them sprite ownership + cleanup, a position, copy-protection and a
// default Render. Motion that needs forces composes a RigidBody on top
// (see Ball); entities that just sit still or move their sprite directly
// don't carry one.
class GameObject {
protected:
    D3DXVECTOR2 position;
    Sprite* sprite;   // owned; deleted by ~GameObject

public:
    GameObject();
    explicit GameObject(Sprite* ownedSprite);
    virtual ~GameObject();

    // Owns a raw Sprite* - copying would double-free
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;

    virtual void Render(LPD3DXSPRITE sharedBrush,
                        D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255));

    Sprite* GetSprite() const { return sprite; }

    // The sprite is the source of truth once it exists (subclasses move it directly)
    D3DXVECTOR2 GetPosition() const { return sprite != nullptr ? sprite->GetPosition() : position; }
    void SetPosition(float x, float y);
};
