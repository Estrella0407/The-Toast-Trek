#include "UiFill.h"
#include <cmath>

namespace ui {

    IDirect3DTexture9* MakeWhiteTexture(IDirect3DDevice9* device) {
        // Just a 1x1 white image loaded from disk (Lecture 3 / Practical 3:
        // D3DXCreateTextureFromFileEx). FillRect() stretches it to any size.
        return LoadTexture(device, "Assets/UI/white.png", 1, 1);
    }

    IDirect3DTexture9* MakeCircleTexture(IDirect3DDevice9* device, UINT size) {
        if (device == NULL || size == 0) return NULL;
        IDirect3DTexture9* tex = NULL;
        if (FAILED(device->CreateTexture(size, size, 1, 0, D3DFMT_A8R8G8B8,
                D3DPOOL_MANAGED, &tex, NULL))) {
            return NULL;
        }
        D3DLOCKED_RECT lr;
        if (SUCCEEDED(tex->LockRect(0, &lr, NULL, 0))) {
            const float c = (size - 1) * 0.5f;
            const float radius = c;
            unsigned char* rows = static_cast<unsigned char*>(lr.pBits);
            for (UINT y = 0; y < size; ++y) {
                DWORD* px = reinterpret_cast<DWORD*>(rows + y * lr.Pitch);
                for (UINT x = 0; x < size; ++x) {
                    const float dx = x - c, dy = y - c;
                    float a = radius - std::sqrt(dx * dx + dy * dy);   // >1 inside, 0..1 at the rim
                    if (a > 1.0f) a = 1.0f;
                    if (a < 0.0f) a = 0.0f;
                    const DWORD alpha = static_cast<DWORD>(a * 255.0f + 0.5f);
                    px[x] = (alpha << 24) | 0x00FFFFFF;
                }
            }
            tex->UnlockRect(0);
        }
        return tex;
    }

    IDirect3DTexture9* LoadTexture(IDirect3DDevice9* device, const char* path,
                                   UINT width, UINT height) {
        if (device == NULL) return NULL;
        IDirect3DTexture9* tex = NULL;
        D3DXCreateTextureFromFileExA(device, path, width, height, 1, 0,
            D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0,
            NULL, NULL, &tex);
        return tex;
    }

    void FillRect(LPD3DXSPRITE brush, IDirect3DTexture9* whiteTex,
                  float x, float y, float w, float h, D3DCOLOR color) {
        if (brush == NULL || whiteTex == NULL || w <= 0.0f || h <= 0.0f) return;

        D3DXVECTOR2 scale(w, h);
        D3DXVECTOR2 translate(x, y);
        D3DXMATRIX transform;
        D3DXMatrixTransformation2D(&transform, NULL, 0.0f, &scale, NULL, 0.0f, &translate);
        brush->SetTransform(&transform);

        RECT src = { 0, 0, 1, 1 };
        brush->Draw(whiteTex, &src, NULL, NULL, color);

        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        brush->SetTransform(&identity);
    }

    void DrawTextureRegion(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                           const RECT& src, float x, float y,
                           float scaleX, float scaleY, D3DCOLOR tint) {
        if (brush == NULL || tex == NULL) return;

        D3DXVECTOR2 scale(scaleX, scaleY);
        D3DXVECTOR2 translate(x, y);
        D3DXMATRIX transform;
        D3DXMatrixTransformation2D(&transform, NULL, 0.0f, &scale, NULL, 0.0f, &translate);
        brush->SetTransform(&transform);

        brush->Draw(tex, &src, NULL, NULL, tint);

        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        brush->SetTransform(&identity);
    }

    void DrawTexture(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                     UINT srcW, UINT srcH, float x, float y,
                     float scaleX, float scaleY, D3DCOLOR tint) {
        RECT src = { 0, 0, (LONG)srcW, (LONG)srcH };
        DrawTextureRegion(brush, tex, src, x, y, scaleX, scaleY, tint);
    }

    void DrawTextureRotated(LPD3DXSPRITE brush, IDirect3DTexture9* tex,
                            UINT srcW, UINT srcH, float centreX, float centreY,
                            float drawW, float drawH, float angleRad,
                            D3DCOLOR tint) {
        if (brush == NULL || tex == NULL || srcW == 0 || srcH == 0) return;

        // Scale the srcW x srcH texture down to drawW x drawH (about the
        // origin), rotate about the scaled quad's centre, then translate so
        // that centre lands on (centreX, centreY).
        D3DXVECTOR2 scale(drawW / (float)srcW, drawH / (float)srcH);
        D3DXVECTOR2 rotCentre(drawW * 0.5f, drawH * 0.5f);
        D3DXVECTOR2 translate(centreX - drawW * 0.5f, centreY - drawH * 0.5f);
        D3DXMATRIX transform;
        D3DXMatrixTransformation2D(&transform, NULL, 0.0f, &scale,
                                   &rotCentre, angleRad, &translate);
        brush->SetTransform(&transform);

        RECT src = { 0, 0, (LONG)srcW, (LONG)srcH };
        brush->Draw(tex, &src, NULL, NULL, tint);

        D3DXMATRIX identity;
        D3DXMatrixIdentity(&identity);
        brush->SetTransform(&identity);
    }
}
