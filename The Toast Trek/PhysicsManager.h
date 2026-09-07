#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"
#include "TileMap.h"
#include "AABB.h"

class PhysicsManager {
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

    // --- Circular-body physics (Lecture 6: "quick collision checker" + "deflect") ---

    // Distance test between two circle centres:
    //   collision  <=>  (radiusA + radiusB) > |centreB - centreA|
    static bool CirclesOverlap(const D3DXVECTOR2& centreA, float radiusA,
                               const D3DXVECTOR2& centreB, float radiusB);

    // Non-axis-aligned elastic collision response for two moving circular
    // bodies of arbitrary mass. Everything is resolved along the
    // centre-to-centre normal n (not the screen axes):
    //   1. separate the overlap, split between the bodies by inverse mass
    //   2. apply an impulse along n
    //        j = -(1 + e) * (relativeVelocity . n) / (1/mA + 1/mB)
    //      vA -= (j/mA) n ,  vB += (j/mB) n
    // `restitution` e is the bounciness (1 = perfectly elastic). A mass <= 0
    // is treated as immovable. No-op if the bodies are already separating.
    // Positions and velocities are updated in place.
    static void ResolveCircleCollision(D3DXVECTOR2& posA, D3DXVECTOR2& velA, float massA, float radiusA,
                                       D3DXVECTOR2& posB, D3DXVECTOR2& velB, float massB, float radiusB,
                                       float restitution = 1.0f);
};
