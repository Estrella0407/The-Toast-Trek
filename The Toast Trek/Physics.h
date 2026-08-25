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

    // A box smaller than the sprite's full canvas, centered horizontally and
    // anchored at the bottom ("feet") - most character art has empty/soft
    // padding above and to the sides of the actual standing pose, so
    // colliding tiles against the full canvas makes gaps that look easily
    // walkable feel blocked. widthRatio/heightRatio are 0..1 fractions of
    // the sprite's full (scaled) size.
    static AABB GetFootBounds(Sprite* sprite, float widthRatio, float heightRatio);

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
    //
    // footWidthRatio/footHeightRatio (each 0..1, default 1 = full sprite
    // bounds) shrink the box used for the check via GetFootBounds() - pass
    // something smaller for characters walking through tile-based maps, so
    // solid tiles only block on the character's actual footprint.
    static bool ResolveCollisionShapes(Sprite* sprite, const TileMap* map,
        float footWidthRatio = 1.0f, float footHeightRatio = 1.0f);
};