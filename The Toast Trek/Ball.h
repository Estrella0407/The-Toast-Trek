#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "GameObject.h"

// A circular body for the physics demo. Position + velocity + mass come from
// GameObject / RigidBody; the ball adds a radius and draws a spinning
// football texture (shared, not owned).
class Ball : public GameObject {
private:
    float radius;
    float mass;
    float angle;   // current spin angle, radians
    float spin;    // spin speed, radians per step
    IDirect3DTexture9* tex;   // Assets/Characters/football.png - shared

public:
    Ball(IDirect3DTexture9* sharedTex, float x, float y, float radius, float mass);

    float Radius() const { return radius; }
    float Mass() const { return mass; }

    // Add `accel` in unit direction `dir` to the velocity, capped at `maxSpeed`.
    void Drive(const D3DXVECTOR2& dir, float accel, float maxSpeed);

    // Integrate one fixed step: move by the velocity, spin, apply friction.
    void Step(float friction);

    void Render(LPD3DXSPRITE brush, D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255)) override;
    AABB GetBounds() const override;
};
