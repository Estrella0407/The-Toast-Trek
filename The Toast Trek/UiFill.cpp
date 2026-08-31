#include "UiFill.h"

namespace ui {

    IDirect3DTexture9* MakeWhiteTexture(IDirect3DDevice9* device) {
        if (device == NULL) return NULL;
        IDirect3DTexture9* tex = NULL;
        if (FAILED(device->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8,
                D3DPOOL_MANAGED, &tex, NULL))) {
            return NULL;
        }
        D3DLOCKED_RECT locked;
        if (SUCCEEDED(tex->LockRect(0, &locked, NULL, 0))) {
            *reinterpret_cast<DWORD*>(locked.pBits) = 0xFFFFFFFF;
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
}
