#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// Simple UI drawing built on the Lecture 3 (2D Graphics) primitives:
//   - filled rectangles / bars / borders : ID3DXLine  (a wide line is a bar)
//   - text                               : the Font class
//   - images / icons                     : ID3DXSprite::Draw of a loaded texture
namespace ui {

    // Shared colours for a HUD text plate: dark panel, gold border, text shadow
    inline constexpr D3DCOLOR kPlate     = D3DCOLOR_ARGB(190, 18, 15, 12);
    inline constexpr D3DCOLOR kPlateEdge = D3DCOLOR_ARGB(220, 216, 184, 128);
    inline constexpr D3DCOLOR kShadow    = D3DCOLOR_ARGB(230, 0, 0, 0);

    // Create / release the one ID3DXLine that FillRect draws through.
    void Init(IDirect3DDevice9* device);
    void Shutdown();

    // A solid line `thickness` px wide from (ax, ay) to (bx, by). Any angle.
    // Safe to call while `brush` has an open ID3DXSprite batch - it pauses
    // and resumes that batch around the line.
    void FillLine(LPD3DXSPRITE brush, float ax, float ay, float bx, float by,
                  float thickness, D3DCOLOR color);

    // Filled w x h rectangle at (x, y), drawn as one wide FillLine.
    void FillRect(LPD3DXSPRITE brush, float x, float y, float w, float h, D3DCOLOR color);

    // Load a texture, optionally resized to width x height as it loads.
    IDirect3DTexture9* LoadTexture(IDirect3DDevice9* device, const char* path,
                                   UINT width, UINT height);

    // Blit a loaded texture through the sprite brush at a position + scale
    // (the same D3DX transform the Sprite class uses in its Draw()).
    void DrawTexture(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                     UINT srcW, UINT srcH, float x, float y,
                     float scaleX, float scaleY,
                     D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));

    // Blit just the `src` texel rectangle (e.g. one frame of a strip).
    void DrawTextureRegion(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                           const RECT& src, float x, float y,
                           float scaleX, float scaleY,
                           D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));

    // Blit the whole texture scaled to drawW x drawH, rotated `angleRad`
    // about its centre, with that centre placed at (centreX, centreY).
    void DrawTextureRotated(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                            UINT srcW, UINT srcH, float centreX, float centreY,
                            float drawW, float drawH, float angleRad,
                            D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));
}
