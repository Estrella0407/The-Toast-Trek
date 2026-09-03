#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// The game just picks the frame that matches the current value, so the fill
// always looks hand-drawn instead of a stretched flat quad
class FrameBar {
public:
    // `path` is the vertical strip; frameW x frameH is one frame
    // The strip is expected to be frameW x (frameH * frameCount)
    FrameBar(IDirect3DDevice9* device, const char* path,
             int frameW, int frameH, int frameCount);
    ~FrameBar();

    bool IsLoaded() const { return texture != NULL; }
    int FrameCount() const { return frameCount; }

    // Draw frame `index` (clamped to [0, frameCount-1]) with its top-left at (x, y), uniformly scaled
    // `tint` multiplies the frame's colours.
    void DrawFrame(LPD3DXSPRITE brush, int index, float x, float y, float scale,
                   D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255)) const;

    // Draw the frame nearest to `ratio` (0..1) across the whole strip
    void DrawRatio(LPD3DXSPRITE brush, float ratio, float x, float y, float scale,
                   D3DCOLOR tint = D3DCOLOR_ARGB(255, 255, 255, 255)) const;

    float FrameWidth() const { return (float)frameW; }
    float FrameHeight() const { return (float)frameH; }

private:
    IDirect3DDevice9* device;   // For point-sampling the strip (crisp pixel art)
    IDirect3DTexture9* texture;
    int frameW;      // Requested frame size (for the caller's reference)
    int frameH;
    int frameCount;
    // Actual loaded texture size - may differ from requested if the device
    // rounded the strip up to a power of two, in which case D3DX scaled the
    // image to fit and the source rects must be derived from THIS size
    UINT texW;
    UINT texH;
};
