#include "Ball.h"
#include <cmath>

static const RECT kBallSrc = { 303, 198, 1024, 931 };

Ball::Ball(IDirect3DTexture9* sharedTex, float x, float y, float radius, float mass, float maxSpeed, float drag)
    : radius(radius), maxSpeed(maxSpeed), drag(drag), angle(0.0f), spin(0.0f), tex(sharedTex)
{
    position = D3DXVECTOR2(x, y);
    body.mass = mass;
}

void Ball::ApplyThrust(const D3DXVECTOR2& dir, float force)
{
    body.ApplyForce(dir * force);   // -> body.acceleration += force / mass
}

void Ball::Step(float dt)
{
    body.Integrate(dt);             // velocity += acceleration * dt; acceleration cleared

    // Cap the speed: acceleration limited by max speed
    const float sp = sqrtf(body.velocity.x * body.velocity.x +
                           body.velocity.y * body.velocity.y);
    if (sp > maxSpeed && sp > 0.0f)
        body.velocity *= (maxSpeed / sp);

    // Friction: always slows a moving body
    body.velocity *= drag;

    position += body.velocity * dt;

    // Spin follows the horizontal travel a little, then eases off
    spin = spin * 0.92f + body.velocity.x * 0.0016f;
    angle += spin;
}

void Ball::Render(LPD3DXSPRITE brush, D3DCOLOR tint)
{
    if (brush == NULL || tex == NULL) return;

    const float srcW = float(kBallSrc.right - kBallSrc.left);
    const float srcH = float(kBallSrc.bottom - kBallSrc.top);
    const float d = radius * 2.0f;                 // on-screen diameter = collision diameter

    // Scale the ball sub-rect to d x d, rotate about that quad's centre,
    // translate so the centre lands on `position`.
    D3DXVECTOR2 scale(d / srcW, d / srcH);
    D3DXVECTOR2 rotCentre(d * 0.5f, d * 0.5f);
    D3DXVECTOR2 translate(position.x - d * 0.5f, position.y - d * 0.5f);
    D3DXMATRIX transform;
    D3DXMatrixTransformation2D(&transform, NULL, 0.0f, &scale,
                               &rotCentre, angle, &translate);
    brush->SetTransform(&transform);

    brush->Draw(tex, &kBallSrc, NULL, NULL, tint);

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    brush->SetTransform(&identity);
}
