#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "AABB.h"

// Minimal 2D rigid body: linear motion under accumulated force. A GameObject
// owns one and asks PhysicsManager to integrate / resolve it.
class RigidBody {
public:
    D3DXVECTOR2 velocity;
    D3DXVECTOR2 acceleration;
    float mass;
    AABB bounds;          // world-space, refreshed by the owner each frame

    RigidBody();

    // v += a*dt ; caller adds v*dt to its position. Clears acceleration.
    void Integrate(float dt);

    // a += force / mass
    void ApplyForce(const D3DXVECTOR2& force);

    void Stop();
};
