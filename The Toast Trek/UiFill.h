#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// Solid-colour rectangles drawn through the shared sprite brush
// Safe way to draw filled quads while Main keeps an ID3DXSprite batch open for the whole frame
// ID3DXLine corrupts that batch and makes every sprite/font drawn afterwards render invisibly
namespace ui {

    // Shared colours for a text plate:
    // dark panel, gold border, text shadow - use these so every HUD readout matches
    inline constexpr D3DCOLOR kPlate     = D3DCOLOR_ARGB(190, 18, 15, 12);
    inline constexpr D3DCOLOR kPlateEdge = D3DCOLOR_ARGB(220, 216, 184, 128);
    inline constexpr D3DCOLOR kShadow    = D3DCOLOR_ARGB(230, 0, 0, 0);


    IDirect3DTexture9* MakeWhiteTexture(IDirect3DDevice9* device);
    IDirect3DTexture9* MakeCircleTexture(IDirect3DDevice9* device, UINT size = 64);
    IDirect3DTexture9* LoadTexture(IDirect3DDevice9* device, const char* path,
                                   UINT width, UINT height);

    // Fill w x h pixels at (x, y) with `color` (alpha honoured)
    // No-op if brush or whiteTex is NULL
    void FillRect(LPD3DXSPRITE brush, IDirect3DTexture9* whiteTex,
                  float x, float y, float w, float h, D3DCOLOR color);

    // Blit `tex` with its top-left corner at (x, y), scaled about the origin
    // (top-left stays put - unlike Sprite, which scales about its Centre)
    void DrawTexture(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                     UINT srcW, UINT srcH, float x, float y,
                     float scaleX, float scaleY,
                     D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));

    // Only the `src` texel rectangle (e.g. one frame of a vertical strip)
    // The frame's top-left lands at (x, y)
    void DrawTextureRegion(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                           const RECT& src, float x, float y,
                           float scaleX, float scaleY,
                           D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));

    // Draw the whole texture scaled to drawW x drawH
    // Rotated `angleRad` about its own centre, with that centre placed at (centreX, centreY)
    void DrawTextureRotated(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                            UINT srcW, UINT srcH, float centreX, float centreY,
                            float drawW, float drawH, float angleRad,
                            D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));
}
