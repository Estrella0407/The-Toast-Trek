#include "Physics.h"
#include <cmath>
#include <cfloat>

namespace {
    void GetSpriteVertices(Sprite* sprite, D3DXVECTOR2 vertices[4]) {
        D3DXVECTOR2 pos = sprite->GetPosition();
        RECT frame = sprite->GetRect();

        float width = (float)(frame.right - frame.left);
        float height = (float)(frame.bottom - frame.top);

        D3DXVECTOR2 local[4] = {
            D3DXVECTOR2(0.0f, 0.0f),
            D3DXVECTOR2(width, 0.0f),
            D3DXVECTOR2(width, height),
            D3DXVECTOR2(0.0f, height)
        };

        D3DXVECTOR2 pivot(width * 0.5f, height * 0.5f);
        D3DXVECTOR2 scale = sprite->GetScale();

        float cosine = cosf(sprite->GetRotation());
        float sine = sinf(sprite->GetRotation());

        for (int i = 0; i < 4; ++i) {
            D3DXVECTOR2 relative = local[i] - pivot;

            relative.x *= scale.x;
            relative.y *= scale.y;

            vertices[i] = D3DXVECTOR2(
                pos.x + pivot.x + relative.x * cosine - relative.y * sine,
                pos.y + pivot.y + relative.x * sine + relative.y * cosine
            );
        }
    }
}

AABB Physics::GetBounds(Sprite* sprite) {
    AABB box = { 0.0f, 0.0f, 0.0f, 0.0f };

    if (sprite == NULL) return box;

    D3DXVECTOR2 vertices[4];
    GetSpriteVertices(sprite, vertices);

    box.left = box.right = vertices[0].x;
    box.top = box.bottom = vertices[0].y;

    for (int i = 1; i < 4; ++i) {
        if (vertices[i].x < box.left) box.left = vertices[i].x;
        if (vertices[i].x > box.right) box.right = vertices[i].x;
        if (vertices[i].y < box.top) box.top = vertices[i].y;
        if (vertices[i].y > box.bottom) box.bottom = vertices[i].y;
    }

    return box;
}

AABB Physics::GetFootBounds(Sprite* sprite, float widthRatio, float heightRatio) {
    AABB full = GetBounds(sprite);

    if (widthRatio >= 1.0f && heightRatio >= 1.0f) return full;

    float fullWidth = full.right - full.left;
    float fullHeight = full.bottom - full.top;

    float footWidth = fullWidth * widthRatio;
    float footHeight = fullHeight * heightRatio;

    AABB box;
    box.left = full.left + (fullWidth - footWidth) * 0.5f;
    box.right = box.left + footWidth;
    box.bottom = full.bottom; // anchored at the feet
    box.top = box.bottom - footHeight;

    return box;
}

AABB Physics::GetHeartBounds(Sprite* sprite) {
    AABB box = { 0.0f, 0.0f, 0.0f, 0.0f };

    if (sprite == NULL) return box;
    D3DXVECTOR2 position = sprite->GetPosition();

    const float offsetX = 11.0f;
    const float offsetY = 16.5f;

    const float heartWidth = 42.0f;
    const float heartHeight = 31.0f;

    box.left = position.x + offsetX;
    box.top = position.y + offsetY;
    box.right = box.left + heartWidth;
    box.bottom = box.top + heartHeight;

    return box;
}

bool Physics::CheckAABBCollision(const AABB& a, const AABB& b) {
    return(a.left < b.right &&
        a.right > b.left &&
        a.top < b.bottom &&
        a.bottom > b.top);
}

void Physics::ClampToBounds(Sprite* sprite, float minX, float minY, float maxX, float maxY) {
    if (sprite == NULL) return;

    D3DXVECTOR2 pos = sprite->GetPosition();
    AABB box = GetBounds(sprite);

    float moveX = 0.0f;
    float moveY = 0.0f;

    if (box.left < minX) moveX = minX - box.left;
    else if (box.right > maxX) moveX = maxX - box.right;

    if (box.top < minY) moveY = minY - box.top;
    else if (box.bottom > maxY) moveY = maxY - box.bottom;

    sprite->SetPosition(pos.x + moveX, pos.y + moveY);
}

bool Physics::ResolveCollisionShapes(Sprite* sprite, const TileMap* map,
    float footWidthRatio, float footHeightRatio) {
    if (sprite == NULL || map == NULL) return false;

    int tileWidth = map->GetTileWidth();
    int tileHeight = map->GetTileHeight();
    if (tileWidth <= 0 || tileHeight <= 0) return false;

    bool collided = false;

    // Resolve against one overlapping tile at a time, re-checking the box
    // after each push so a correction against one tile can't leave the
    // sprite still stuck in a neighboring one. A handful of passes is
    // plenty for how few tiles a sprite's small box can touch at once.
    const int kMaxPasses = 4;

    for (int pass = 0; pass < kMaxPasses; ++pass) {
        AABB box = GetFootBounds(sprite, footWidthRatio, footHeightRatio);

        int minTileX = (int)floorf(box.left / (float)tileWidth);
        int maxTileX = (int)floorf((box.right - 0.0001f) / (float)tileWidth);
        int minTileY = (int)floorf(box.top / (float)tileHeight);
        int maxTileY = (int)floorf((box.bottom - 0.0001f) / (float)tileHeight);

        bool resolvedThisPass = false;

        for (int tileY = minTileY; tileY <= maxTileY && !resolvedThisPass; ++tileY) {
            for (int tileX = minTileX; tileX <= maxTileX && !resolvedThisPass; ++tileX) {
                if (!map->IsTileSolid(tileX, tileY)) continue;

                float tileLeft = (float)(tileX * tileWidth);
                float tileTop = (float)(tileY * tileHeight);
                float tileRight = tileLeft + tileWidth;
                float tileBottom = tileTop + tileHeight;

                // Real overlap on both axes - a tile only counts once the
                // sprite's box is actually touching it, never "pixels away".
                float overlapRight = box.right < tileRight ? box.right : tileRight;
                float overlapLeft = box.left > tileLeft ? box.left : tileLeft;
                float overlapX = overlapRight - overlapLeft;

                float overlapBottom = box.bottom < tileBottom ? box.bottom : tileBottom;
                float overlapTop = box.top > tileTop ? box.top : tileTop;
                float overlapY = overlapBottom - overlapTop;

                if (overlapX <= 0.0f || overlapY <= 0.0f) continue; // not actually touching

                D3DXVECTOR2 position = sprite->GetPosition();

                // Push out along whichever axis needs the smaller nudge.
                if (overlapX < overlapY) {
                    float boxCenterX = (box.left + box.right) * 0.5f;
                    float tileCenterX = tileLeft + tileWidth * 0.5f;
                    float sign = (boxCenterX < tileCenterX) ? -1.0f : 1.0f;
                    sprite->SetPosition(position.x + sign * overlapX, position.y);
                }
                else {
                    float boxCenterY = (box.top + box.bottom) * 0.5f;
                    float tileCenterY = tileTop + tileHeight * 0.5f;
                    float sign = (boxCenterY < tileCenterY) ? -1.0f : 1.0f;
                    sprite->SetPosition(position.x, position.y + sign * overlapY);
                }

                collided = true;
                resolvedThisPass = true;
            }
        }

        if (!resolvedThisPass) break;
    }

    return collided;
}

// ===================================================================
//  Convex-polygon rigid bodies: SAT collision + impulse response
// ===================================================================

namespace {
    inline float Len2D(float x, float y) { return std::sqrt(x * x + y * y); }

    // Vertex of `v` furthest along `dir` - the contact point for the torque.
    D3DXVECTOR2 SupportPoint(const D3DXVECTOR2* v, int n, const D3DXVECTOR2& dir) {
        int best = 0; float bestD = -FLT_MAX;
        for (int i = 0; i < n; ++i) {
            const float d = v[i].x * dir.x + v[i].y * dir.y;
            if (d > bestD) { bestD = d; best = i; }
        }
        return v[best];
    }
}

void Physics::MakeBox(Body& b, float cx, float cy, float hw, float hh,
    float invMass, float invInertia) {
    b.pos = D3DXVECTOR2(cx, cy);
    b.vel = D3DXVECTOR2(0.0f, 0.0f);
    b.angle = 0.0f; b.angVel = 0.0f;
    b.invMass = invMass; b.invInertia = invInertia;
    b.local[0] = D3DXVECTOR2(-hw, -hh);
    b.local[1] = D3DXVECTOR2(hw, -hh);
    b.local[2] = D3DXVECTOR2(hw, hh);
    b.local[3] = D3DXVECTOR2(-hw, hh);
    b.count = 4;
}

void Physics::MakePolygon(Body& b, float cx, float cy, float r, int sides,
    float mass) {
    if (sides < 3) sides = 3;
    if (sides > 8) sides = 8;
    b.pos = D3DXVECTOR2(cx, cy);
    b.vel = D3DXVECTOR2(0.0f, 0.0f);
    b.angle = 0.0f; b.angVel = 0.0f;
    b.invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    // moment of inertia approximated as a solid disc: I = 1/2 m r^2
    const float I = 0.5f * mass * r * r;
    b.invInertia = (I > 0.0f) ? 1.0f / I : 0.0f;
    const float step = 6.28318530718f / (float)sides;
    for (int i = 0; i < sides; ++i) {
        const float a = i * step + step * 0.5f;   // flat side facing the axes
        b.local[i] = D3DXVECTOR2(r * std::cos(a), r * std::sin(a));
    }
    b.count = sides;
}

void Physics::BodyWorldVertices(const Body& b, D3DXVECTOR2* out) {
    const float c = std::cos(b.angle), s = std::sin(b.angle);
    for (int i = 0; i < b.count; ++i) {
        const D3DXVECTOR2 v = b.local[i];
        out[i] = D3DXVECTOR2(b.pos.x + v.x * c - v.y * s,
                             b.pos.y + v.x * s + v.y * c);
    }
}

bool Physics::SatPolyPoly(const D3DXVECTOR2* va, int na,
    const D3DXVECTOR2* vb, int nb,
    const D3DXVECTOR2& centreA, const D3DXVECTOR2& centreB,
    D3DXVECTOR2& normal, float& depth) {
    depth = FLT_MAX;
    // Every edge normal of both polygons is a candidate separating axis.
    for (int poly = 0; poly < 2; ++poly) {
        const D3DXVECTOR2* v = (poly == 0) ? va : vb;
        const int n = (poly == 0) ? na : nb;
        for (int i = 0; i < n; ++i) {
            const D3DXVECTOR2 e = { v[(i + 1) % n].x - v[i].x,
                                    v[(i + 1) % n].y - v[i].y };
            const float el = Len2D(e.x, e.y);
            if (el < 1e-6f) continue;
            const D3DXVECTOR2 ax = { -e.y / el, e.x / el };   // edge normal

            // project both polygons onto this axis (dot product)
            float minA = FLT_MAX, maxA = -FLT_MAX, minB = FLT_MAX, maxB = -FLT_MAX;
            for (int j = 0; j < na; ++j) {
                const float d = va[j].x * ax.x + va[j].y * ax.y;
                minA = fminf(minA, d); maxA = fmaxf(maxA, d);
            }
            for (int j = 0; j < nb; ++j) {
                const float d = vb[j].x * ax.x + vb[j].y * ax.y;
                minB = fminf(minB, d); maxB = fmaxf(maxB, d);
            }

            // compare the intervals: a non-positive overlap means a gap
            const float overlap = fminf(maxA, maxB) - fmaxf(minA, minB);
            if (overlap <= 0.0f) return false;          // separated - done
            if (overlap < depth) { depth = overlap; normal = ax; }
        }
    }
    // orient the normal so it points from A towards B
    const D3DXVECTOR2 ab = { centreB.x - centreA.x, centreB.y - centreA.y };
    if (ab.x * normal.x + ab.y * normal.y < 0.0f) {
        normal.x = -normal.x; normal.y = -normal.y;
    }
    return true;
}

void Physics::ResolveBodies(Body& a, Body& b, const D3DXVECTOR2& n,
    float depth, float e, const D3DXVECTOR2* aWorld, int aCount) {
    const float invSum = a.invMass + b.invMass;
    if (invSum <= 0.0f) return;

    // --- overlap resolution (MTV): separate along the contact normal,
    //     shared out by inverse mass.
    a.pos.x -= n.x * depth * (a.invMass / invSum);
    a.pos.y -= n.y * depth * (a.invMass / invSum);
    b.pos.x += n.x * depth * (b.invMass / invSum);
    b.pos.y += n.y * depth * (b.invMass / invSum);

    // --- contact point: A's leading vertex (the one buried in B).
    const D3DXVECTOR2 contact = SupportPoint(aWorld, aCount, n);
    const D3DXVECTOR2 rA = { contact.x - a.pos.x, contact.y - a.pos.y };
    const D3DXVECTOR2 rB = { contact.x - b.pos.x, contact.y - b.pos.y };

    // relative velocity at the contact (linear + omega x r)
    const D3DXVECTOR2 vA = { a.vel.x - a.angVel * rA.y, a.vel.y + a.angVel * rA.x };
    const D3DXVECTOR2 vB = { b.vel.x - b.angVel * rB.y, b.vel.y + b.angVel * rB.x };
    const float vn = (vB.x - vA.x) * n.x + (vB.y - vA.y) * n.y;
    if (vn > 0.0f) return;   // already separating

    const float rAxn = rA.x * n.y - rA.y * n.x;
    const float rBxn = rB.x * n.y - rB.y * n.x;
    const float denom = invSum
        + rAxn * rAxn * a.invInertia
        + rBxn * rBxn * b.invInertia;

    // j = -(1 + e)(vRel . n) / denom      (F = m a  ->  dv = j / m)
    const float j = -(1.0f + e) * vn / denom;
    const D3DXVECTOR2 P = { n.x * j, n.y * j };

    a.vel.x -= P.x * a.invMass;  a.vel.y -= P.y * a.invMass;
    b.vel.x += P.x * b.invMass;  b.vel.y += P.y * b.invMass;
    a.angVel -= rAxn * j * a.invInertia;
    b.angVel += rBxn * j * b.invInertia;
}