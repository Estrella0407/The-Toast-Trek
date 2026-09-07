#include "PhysicsManager.h"
#include <cmath>

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

AABB PhysicsManager::GetBounds(Sprite* sprite) {
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

AABB PhysicsManager::GetFootBounds(Sprite* sprite, float widthRatio, float heightRatio) {
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

AABB PhysicsManager::GetHeartBounds(Sprite* sprite) {
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

bool PhysicsManager::CheckAABBCollision(const AABB& a, const AABB& b) {
    return(a.left < b.right &&
        a.right > b.left &&
        a.top < b.bottom &&
        a.bottom > b.top);
}

void PhysicsManager::ClampToBounds(Sprite* sprite, float minX, float minY, float maxX, float maxY) {
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

bool PhysicsManager::ResolveCollisionShapes(Sprite* sprite, const TileMap* map,
    float footWidthRatio, float footHeightRatio) {
    if (sprite == NULL || map == NULL) return false;

    int tileWidth = map->GetTileWidth();
    int tileHeight = map->GetTileHeight();
    if (tileWidth <= 0 || tileHeight <= 0) return false;

    bool collided = false;

    // Resolve one overlapping tile at a time, re-checking after each push
    // to prevent the sprite from remaining stuck in adjacent tiles
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

                // Real overlap on both axes
                // A tile only counts once the sprite's box is actually touching it
                float overlapRight = box.right < tileRight ? box.right : tileRight;
                float overlapLeft = box.left > tileLeft ? box.left : tileLeft;
                float overlapX = overlapRight - overlapLeft;

                float overlapBottom = box.bottom < tileBottom ? box.bottom : tileBottom;
                float overlapTop = box.top > tileTop ? box.top : tileTop;
                float overlapY = overlapBottom - overlapTop;

                if (overlapX <= 0.0f || overlapY <= 0.0f) continue; // Not actually touching

                D3DXVECTOR2 position = sprite->GetPosition();

                // Push out along whichever axis needs the smaller nudge
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
// --- Circular-body physics -------------------------------------------------

bool PhysicsManager::CirclesOverlap(const D3DXVECTOR2& centreA, float radiusA,
                                    const D3DXVECTOR2& centreB, float radiusB) {
    const float dx = centreB.x - centreA.x;
    const float dy = centreB.y - centreA.y;
    const float distSq = dx * dx + dy * dy;
    const float r = radiusA + radiusB;
    return distSq < r * r;
}

void PhysicsManager::ResolveCircleCollision(D3DXVECTOR2& posA, D3DXVECTOR2& velA, float massA, float radiusA,
                                            D3DXVECTOR2& posB, D3DXVECTOR2& velB, float massB, float radiusB,
                                            float restitution) {
    // Vector from A to B, and the distance between the centres
    D3DXVECTOR2 delta = posB - posA;
    float dist = sqrtf(delta.x * delta.x + delta.y * delta.y);

    // Exactly overlapping centres - shove them apart on an arbitrary axis
    if (dist <= 1e-4f) {
        delta = D3DXVECTOR2(1.0f, 0.0f);
        dist = 1.0f;
    }

    const float overlap = (radiusA + radiusB) - dist;
    if (overlap <= 0.0f) return;                 // not actually touching

    const D3DXVECTOR2 n = delta / dist;          // collision normal, A -> B

    const float invA = massA > 0.0f ? 1.0f / massA : 0.0f;
    const float invB = massB > 0.0f ? 1.0f / massB : 0.0f;
    const float invSum = invA + invB;
    if (invSum <= 0.0f) return;                  // both immovable

    // 1. Positional correction - separate the overlap, split by inverse mass
    posA -= n * (overlap * (invA / invSum));
    posB += n * (overlap * (invB / invSum));

    // 2. Impulse along the normal
    const D3DXVECTOR2 relVel = velB - velA;
    const float velAlongN = relVel.x * n.x + relVel.y * n.y;
    if (velAlongN > 0.0f) return;                // already separating

    const float j = -(1.0f + restitution) * velAlongN / invSum;
    const D3DXVECTOR2 impulse = n * j;
    velA -= impulse * invA;
    velB += impulse * invB;
}

// --- Separating Axis Theorem ---------------------------------------------------

namespace {
    // [min, max] of a polygon projected onto a (unit) axis - the dot products
    void ProjectPolygon(const D3DXVECTOR2* v, int n, const D3DXVECTOR2& axis,
                        float& outMin, float& outMax) {
        outMin = outMax = v[0].x * axis.x + v[0].y * axis.y;
        for (int i = 1; i < n; ++i) {
            const float p = v[i].x * axis.x + v[i].y * axis.y;
            if (p < outMin) outMin = p;
            if (p > outMax) outMax = p;
        }
    }

    // Overlap of [aMin,aMax] and [bMin,bMax]; <= 0 means a separating gap
    float IntervalOverlap(float aMin, float aMax, float bMin, float bMax) {
        const float hi = aMax < bMax ? aMax : bMax;
        const float lo = aMin > bMin ? aMin : bMin;
        return hi - lo;
    }
}

void PhysicsManager::BoxCorners(const D3DXVECTOR2& centre, float halfWidth, float halfHeight,
                                float angleRad, D3DXVECTOR2 outCorners[4]) {
    const float c = cosf(angleRad), s = sinf(angleRad);
    const D3DXVECTOR2 local[4] = {
        D3DXVECTOR2(-halfWidth, -halfHeight),
        D3DXVECTOR2( halfWidth, -halfHeight),
        D3DXVECTOR2( halfWidth,  halfHeight),
        D3DXVECTOR2(-halfWidth,  halfHeight)
    };
    for (int i = 0; i < 4; ++i) {
        outCorners[i] = D3DXVECTOR2(
            centre.x + local[i].x * c - local[i].y * s,
            centre.y + local[i].x * s + local[i].y * c);
    }
}

bool PhysicsManager::SatOverlap(const D3DXVECTOR2* polyA, int countA,
                                const D3DXVECTOR2* polyB, int countB,
                                D3DXVECTOR2* outAxis, float* outDepth) {
    if (countA < 3 || countB < 3) return false;

    float bestDepth = 1e30f;
    D3DXVECTOR2 bestAxis(0.0f, 0.0f);

    // Test the edge normals of both polygons
    for (int poly = 0; poly < 2; ++poly) {
        const D3DXVECTOR2* v = poly == 0 ? polyA : polyB;
        const int n = poly == 0 ? countA : countB;

        for (int i = 0; i < n; ++i) {
            const D3DXVECTOR2 edge = v[(i + 1) % n] - v[i];
            D3DXVECTOR2 axis(-edge.y, edge.x);          // perpendicular = the face normal
            const float len = sqrtf(axis.x * axis.x + axis.y * axis.y);
            if (len < 1e-6f) continue;                  // degenerate edge
            axis /= len;

            float aMin, aMax, bMin, bMax;
            ProjectPolygon(polyA, countA, axis, aMin, aMax);
            ProjectPolygon(polyB, countB, axis, bMin, bMax);

            const float overlap = IntervalOverlap(aMin, aMax, bMin, bMax);
            if (overlap <= 0.0f) return false;          // separating axis -> no collision
            if (overlap < bestDepth) { bestDepth = overlap; bestAxis = axis; }
        }
    }

    if (outAxis)  *outAxis = bestAxis;
    if (outDepth) *outDepth = bestDepth;
    return true;
}

bool PhysicsManager::SatCircleVsPolygon(const D3DXVECTOR2& centre, float radius,
                                        const D3DXVECTOR2* poly, int count,
                                        D3DXVECTOR2* outAxis, float* outDepth) {
    if (count < 3) return false;

    float bestDepth = 1e30f;
    D3DXVECTOR2 bestAxis(0.0f, 0.0f);

    // Helper: test one candidate axis, updating the best (MTV) so far
    auto testAxis = [&](D3DXVECTOR2 axis) -> bool {
        const float len = sqrtf(axis.x * axis.x + axis.y * axis.y);
        if (len < 1e-6f) return true;                   // skip, not separating
        axis /= len;

        float pMin, pMax;
        ProjectPolygon(poly, count, axis, pMin, pMax);
        const float c = centre.x * axis.x + centre.y * axis.y;
        const float overlap = IntervalOverlap(pMin, pMax, c - radius, c + radius);
        if (overlap <= 0.0f) return false;              // separating axis found
        if (overlap < bestDepth) { bestDepth = overlap; bestAxis = axis; }
        return true;
    };

    // 1. polygon edge normals
    for (int i = 0; i < count; ++i) {
        const D3DXVECTOR2 edge = poly[(i + 1) % count] - poly[i];
        if (!testAxis(D3DXVECTOR2(-edge.y, edge.x))) return false;
    }

    // 2. closest polygon vertex -> circle centre (the axis SAT alone misses)
    int closest = 0;
    float bestD2 = 1e30f;
    for (int i = 0; i < count; ++i) {
        const float dx = poly[i].x - centre.x, dy = poly[i].y - centre.y;
        const float d2 = dx * dx + dy * dy;
        if (d2 < bestD2) { bestD2 = d2; closest = i; }
    }
    if (!testAxis(poly[closest] - centre)) return false;

    if (outAxis)  *outAxis = bestAxis;
    if (outDepth) *outDepth = bestDepth;
    return true;
}
