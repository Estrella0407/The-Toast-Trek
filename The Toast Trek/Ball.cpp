#include "Ball.h"
#include "UiFill.h"
#include <cmath>

// football.png source pixel size
static const UINT kTexW = 1330;
static const UINT kTexH = 1183;

Ball::Ball(IDirect3DTexture9* sharedTex, float x, float y, float radius, float mass)
    : radius(radius), mass(mass), angle(0.0f), spin(0.0f), tex(sharedTex)
{
    position = D3DXVECTOR2(x, y);
    body.mass = mass;
}

void Ball::Drive(const D3DXVECTOR2& dir, float accel, float maxSpeed)
{
    body.velocity += dir * accel;

    const float sp = sqrtf(body.velocity.x * body.velocity.x +
                           body.velocity.y * body.velocity.y);
    if (sp > maxSpeed && sp > 0.0f)
        body.velocity *= (maxSpeed / sp);
}

void Ball::Step(float friction)
{
    position += body.velocity;

    // Spin follows the horizontal travel a little, then eases off
    spin = spin * 0.92f + body.velocity.x * 0.0016f;
    angle += spin;

    body.velocity *= friction;
}

void Ball::Render(LPD3DXSPRITE brush, D3DCOLOR tint)
{
    const float d = radius * 2.0f;
    ui::DrawTextureRotated(brush, tex, kTexW, kTexH,
                           position.x, position.y, d, d, angle, tint);
}

AABB Ball::GetBounds() const
{
    return AABB{ position.x - radius, position.y - radius,
                position.x + radius, position.y + radius };
}
