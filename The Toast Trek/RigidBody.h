#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// Minimal 2D rigid body: linear motion under accumulated force. Composed by
// the entities that need force-based movement (currently Ball); the scene
// applies forces, then calls Integrate() once per step.
class RigidBody {
public:
    D3DXVECTOR2 velocity;
    D3DXVECTOR2 acceleration;
    float mass;

    RigidBody();

    // v += a*dt ; caller adds v*dt to its position. Clears acceleration.
    void Integrate(float dt);

    // a += force / mass
    void ApplyForce(const D3DXVECTOR2& force);

    void Stop();
};
