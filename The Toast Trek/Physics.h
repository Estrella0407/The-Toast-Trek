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

    // ----- Convex-polygon rigid bodies: Separating Axis Theorem + impulse
    // The AABB helpers above cover the tile / heart / projectile checks in
    // the rest of the game. This block is the full physics demo used by the
    // ending's bouncing-ball credit roll:
    //   1. SatPolyPoly    - collision detection (SAT: project onto every
    //                       edge normal, compare the intervals, least-
    //                       overlap axis is the contact normal; convex only).
    //   2. ResolveBodies  - overlap resolution (push apart by the MTV) then
    //                       a bounce impulse, equal and opposite so momentum
    //                       is conserved (F = m a; an impulse is F*dt). The
    //                       (r x n) terms give the spin from an off-centre hit.
    struct Body {
        D3DXVECTOR2 pos;       // centre of mass, world space
        D3DXVECTOR2 vel;
        float angle;
        float angVel;
        float invMass;         // 0 => immovable (infinite mass)
        float invInertia;
        D3DXVECTOR2 local[8];  // vertices about the centre, body space
        int count;
    };

    // Fill `b` with an axis-aligned box (centre cx,cy; half-extents hw,hh).
    static void MakeBox(Body& b, float cx, float cy, float hw, float hh,
        float invMass = 0.0f, float invInertia = 0.0f);

    // Fill `b` with a regular polygon (3..8 sides) of circum-radius r and
    // mass m; the moment of inertia is approximated as a solid disc.
    static void MakePolygon(Body& b, float cx, float cy, float r, int sides,
        float mass = 1.0f);

    // World-space vertices of `b` (rotated + translated). `out` must hold
    // at least b.count entries.
    static void BodyWorldVertices(const Body& b, D3DXVECTOR2* out);

    // SAT test between two convex vertex loops. On overlap sets `normal`
    // (unit, pointing from A toward B) and `depth` (penetration) and
    // returns true; returns false the instant any axis shows a gap.
    static bool SatPolyPoly(const D3DXVECTOR2* va, int na,
        const D3DXVECTOR2* vb, int nb,
        const D3DXVECTOR2& centreA, const D3DXVECTOR2& centreB,
        D3DXVECTOR2& normal, float& depth);

    // Resolve a confirmed overlap: separate along `normal` by `depth`
    // (MTV, shared by inverse mass), then apply a restitution-`e` bounce
    // impulse to both bodies. `aWorld`/`aCount` are A's world vertices,
    // used to pick the contact point for the spin term.
    static void ResolveBodies(Body& a, Body& b, const D3DXVECTOR2& normal,
        float depth, float e, const D3DXVECTOR2* aWorld, int aCount);
};