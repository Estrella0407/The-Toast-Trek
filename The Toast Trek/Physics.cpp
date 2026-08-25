#include "Physics.h"
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