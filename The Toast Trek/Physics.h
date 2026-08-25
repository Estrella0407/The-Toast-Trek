#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"
#include "TileMap.h"

struct AABB {
    float left, top, right, bottom;
};

class Physics {
public:
    static AABB GetBounds(Sprite* sprite);
    // Keeps "sprite" fully inside [minX, minY, maxX, maxY] (e.g. the 1280x720
    // window, or the edges of the loaded map). Call this after moving a
    // sprite so it can't walk off the background.

    //Heart hitbox accurately
    static AABB GetHeartBounds(Sprite* sprite);

    //Heart and Projectile collision
    static bool CheckAABBCollision(const AABB& a, const AABB& b);

    static void ClampToBounds(Sprite* sprite, float minX, float minY, float maxX, float maxY);

    // Resolves "sprite" against whichever tiles the map has marked solid via
    // TileMap::SetSolidLayers() - a simple per-tile AABB check, no Tiled
    // Tile Collision Editor shapes required. Only pushes the sprite out when
    // its box actually overlaps a solid tile (never for tiles nearby that
    // aren't touched), nudging it out along whichever axis needs the
    // smaller correction. Returns true if any push happened.
    static bool ResolveCollisionShapes(Sprite* sprite, const TileMap* map);
};