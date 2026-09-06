#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "AABB.h"
#include "RigidBody.h"
#include "Sprite.h"

// Base class for anything that lives in the world with a position, a sprite
// and (optionally) a rigid body. Concrete entities - the player, items,
// enemies, hearts, projectiles - derive from this.
class GameObject {
protected:
    D3DXVECTOR2 position;
    Sprite* sprite;   // owned; deleted by ~GameObject
    RigidBody body;

public:
    GameObject();
    explicit GameObject(Sprite* ownedSprite);
    virtual ~GameObject();

    virtual void Update();
    virtual void Render(LPD3DXSPRITE sharedBrush,
                        D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255));

    virtual AABB GetBounds() const;

    Sprite* GetSprite() const { return sprite; }
    RigidBody& Body() { return body; }

    D3DXVECTOR2 GetPosition() const { return position; }
    void SetPosition(float x, float y);
};
