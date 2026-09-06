#include "RigidBody.h"

RigidBody::RigidBody()
    : velocity(0.0f, 0.0f), acceleration(0.0f, 0.0f), mass(1.0f), bounds{ 0, 0, 0, 0 }
{
}

void RigidBody::Integrate(float dt)
{
    velocity += acceleration * dt;
    acceleration = D3DXVECTOR2(0.0f, 0.0f);
}

void RigidBody::ApplyForce(const D3DXVECTOR2& force)
{
    if (mass > 0.0f) acceleration += force / mass;
}

void RigidBody::Stop()
{
    velocity = D3DXVECTOR2(0.0f, 0.0f);
    acceleration = D3DXVECTOR2(0.0f, 0.0f);
}
