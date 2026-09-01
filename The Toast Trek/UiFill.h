#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// Solid-colour rectangles drawn through the shared sprite brush. This is
// the safe way to draw filled quads while Main keeps an ID3DXSprite batch
// open for the whole frame - ID3DXLine corrupts that batch and makes every
// sprite/font drawn afterwards render invisibly.
namespace ui {

    // Standard palette for a text/readout plate drawn with the 1x1 white
    // texture: a dark translucent panel, a thin gold border one pixel
    // proud of it, and a near-black drop shadow for the text on top.
    // Use these everywhere a FillRect plate backs UI text so every HUD
    // element reads as one set.
    inline constexpr D3DCOLOR kPlate     = D3DCOLOR_ARGB(190, 18, 15, 12);
    inline constexpr D3DCOLOR kPlateEdge = D3DCOLOR_ARGB(220, 216, 184, 128);
    inline constexpr D3DCOLOR kShadow    = D3DCOLOR_ARGB(230, 0, 0, 0);

    // Loads Assets/UI/white.png (a 1x1 opaque-white image) with
    // D3DXCreateTextureFromFileEx - the FillRect helper stretches it into
    // every solid-colour panel. NULL on failure. Caller owns it (Release
    // when done).
    IDirect3DTexture9* MakeWhiteTexture(IDirect3DDevice9* device);

    // A soft-edged white disc, `size` x `size`, alpha 255 inside fading to 0
    // at the rim. Draw it with DrawTexture and a tint for a round ball / dot.
    // NULL on failure. Caller owns it (Release when done).
    IDirect3DTexture9* MakeCircleTexture(IDirect3DDevice9* device, UINT size = 64);

    // Load a texture at its exact pixel size (no power-of-two rescale, so
    // source rects stay accurate). NULL on failure. Caller owns it.
    IDirect3DTexture9* LoadTexture(IDirect3DDevice9* device, const char* path,
                                   UINT width, UINT height);

    // Fill w x h pixels at (x, y) with `color` (alpha honoured). No-op if
    // brush or whiteTex is NULL. Leaves the brush transform as identity.
    void FillRect(LPD3DXSPRITE brush, IDirect3DTexture9* whiteTex,
                  float x, float y, float w, float h, D3DCOLOR color);

    // Blit `tex` with its top-left corner at (x, y), scaled about the
    // origin (so top-left stays put - unlike Sprite, which scales about its
    // centre). Leaves the brush transform as identity.
    void DrawTexture(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                     UINT srcW, UINT srcH, float x, float y,
                     float scaleX, float scaleY,
                     D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));

    // As DrawTexture, but only the `src` texel rectangle (e.g. one frame of
    // a vertical strip). The frame's top-left lands at (x, y).
    void DrawTextureRegion(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                           const RECT& src, float x, float y,
                           float scaleX, float scaleY,
                           D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));

    // Draw the whole texture scaled to drawW x drawH, rotated `angleRad`
    // about its own centre, with that centre placed at (centreX, centreY).
    // Leaves the brush transform as identity.
    void DrawTextureRotated(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                            UINT srcW, UINT srcH, float centreX, float centreY,
                            float drawW, float drawH, float angleRad,
                            D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255));
}
