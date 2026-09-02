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
    // Screen-space bounding box of a sprite (position + scaled frame size)
    static AABB GetBounds(Sprite* sprite);

    // A smaller box at the sprite's feet; widthRatio/heightRatio are 0..1
    // fractions of the full box, so tile collision only blocks on the character's footprint, not the empty art padding
    static AABB GetFootBounds(Sprite* sprite, float widthRatio, float heightRatio);

    // Tight box around the heart's visible pixels
    static AABB GetHeartBounds(Sprite* sprite);

    // True if two boxes overlap
    static bool CheckAABBCollision(const AABB& a, const AABB& b);

    // Keeps the sprite fully inside [minX, minY, maxX, maxY]
    static void ClampToBounds(Sprite* sprite, float minX, float minY, float maxX, float maxY);

    // Pushes the sprite out of any solid tile (SetSolidLayers) it overlaps, along the shorter axis
    // footWidthRatio/footHeightRatio shrink the test box via GetFootBounds
    // Returns true if the sprite was moved
    static bool ResolveCollisionShapes(Sprite* sprite, const TileMap* map,
        float footWidthRatio = 1.0f, float footHeightRatio = 1.0f);
};
