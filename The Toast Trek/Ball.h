#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "GameObject.h"

// A circular rigid body for the physics demo. Position, velocity, mass and
// the force -> acceleration -> velocity chain all live in the inherited
// RigidBody (GameObject::Body()). Each frame the scene applies a thrust
// force, then Step() integrates it and moves the ball. A spinning football
// texture (shared, not owned) is drawn on top - the spin is cosmetic.
class Ball : public GameObject {
private:
    float radius;
    float maxSpeed;   // velocity magnitude is capped here after integration
    float drag;       // per-step velocity damping (Lecture 6/9 friction)
    float angle;      // current spin angle, radians (visual only)
    float spin;       // spin speed, radians per step (visual only)
    IDirect3DTexture9* tex;   // Assets/Characters/football.png - shared

public:
    Ball(IDirect3DTexture9* sharedTex, float x, float y,
         float radius, float mass, float maxSpeed, float drag);

    float Radius() const { return radius; }
    float Mass() const { return body.mass; }

    // Push the body with a force in unit direction `dir` (F = m a, so a
    // heavier ball accelerates less for the same force).
    void ApplyThrust(const D3DXVECTOR2& dir, float force);

    // Integrate one fixed step: RigidBody::Integrate -> cap speed -> drag ->
    // move by the velocity -> spin the texture.
    void Step(float dt);

    void Render(LPD3DXSPRITE brush, D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255)) override;
    AABB GetBounds() const override;
};
