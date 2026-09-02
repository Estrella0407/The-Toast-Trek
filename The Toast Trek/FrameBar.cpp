#include "FrameBar.h"
#include "UiFill.h"
#include <algorithm>
#include <cmath>

FrameBar::FrameBar(IDirect3DDevice9* device, const char* path,
                   int frameW, int frameH, int frameCount)
    : device(device), texture(NULL), frameW(frameW), frameH(frameH),
      frameCount(frameCount > 0 ? frameCount : 1), texW(0), texH(0) {
    texture = ui::LoadTexture(device, path,
                              (UINT)frameW, (UINT)(frameH * this->frameCount));
    if (texture != NULL) {
        D3DSURFACE_DESC desc;
        if (SUCCEEDED(texture->GetLevelDesc(0, &desc))) {
            texW = desc.Width;
            texH = desc.Height;
        }
        if (texW == 0) texW = (UINT)frameW;
        if (texH == 0) texH = (UINT)(frameH * this->frameCount);
    }
}

FrameBar::~FrameBar() {
    if (texture != NULL) texture->Release();
}

void FrameBar::DrawFrame(LPD3DXSPRITE brush, int index, float x, float y, float scale,
                        D3DCOLOR tint) const {
    if (texture == NULL) return;
    index = std::clamp(index, 0, frameCount - 1);

    // Nearest-neighbour so the downscaled pixel-art stays crisp and adjacent
    // strip frames can't bleed into each other along the top/bottom edge
    if (device != NULL) {
        device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    }

    const long long fh = (long long)texH / frameCount;
    const LONG top = (LONG)(index * fh);
    const LONG bottom = (LONG)((index + 1) * fh);
    RECT src = { 0, top, (LONG)texW, bottom };

    // Compensate the on-screen scale so one frame still lands at the size
    // the caller expects (frameW x frameH) * scale.
    const float sx = scale * (float)frameW / (float)texW;
    const float sy = scale * (float)frameH / (float)fh;
    ui::DrawTextureRegion(brush, texture, src, x, y, sx, sy, tint);
}

void FrameBar::DrawRatio(LPD3DXSPRITE brush, float ratio, float x, float y, float scale,
                        D3DCOLOR tint) const {
    ratio = std::clamp(ratio, 0.0f, 1.0f);
    const int index = (int)std::lround(ratio * (frameCount - 1));
    DrawFrame(brush, index, x, y, scale, tint);
}
