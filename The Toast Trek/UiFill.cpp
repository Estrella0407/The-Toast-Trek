#include "UiFill.h"

namespace ui {

    // One shared line, created once at start-up (Init) and freed at shutdown.
    static LPD3DXLINE g_line = NULL;

    void Init(IDirect3DDevice9* device) {
        if (g_line == NULL && device != NULL) {
            D3DXCreateLine(device, &g_line);
        }
    }

    void Shutdown() {
        if (g_line != NULL) {
            g_line->Release();
            g_line = NULL;
        }
    }

    void FillLine(LPD3DXSPRITE brush, float ax, float ay, float bx, float by,
                  float thickness, D3DCOLOR color) {
        if (g_line == NULL || thickness <= 0.0f) return;

        // Drawing lines while an ID3DXSprite batch is open corrupts it, so
        // pause the batch, draw, then resume it.
        const bool inBatch = (brush != NULL);
        if (inBatch) brush->End();

        D3DXVECTOR2 seg[2] = { D3DXVECTOR2(ax, ay), D3DXVECTOR2(bx, by) };
        g_line->SetWidth(thickness);
        g_line->SetAntialias(FALSE);
        g_line->Begin();
        g_line->Draw(seg, 2, color);
        g_line->End();

        if (inBatch) brush->Begin(D3DXSPRITE_ALPHABLEND);
    }

    void FillRect(LPD3DXSPRITE brush, float x, float y, float w, float h, D3DCOLOR color) {
        if (w <= 0.0f || h <= 0.0f) return;

        // One wide line down the longer axis, so the line width never has to
        // exceed the shorter side.
        if (w >= h)
            FillLine(brush, x, y + h * 0.5f, x + w, y + h * 0.5f, h, color);
        else
            FillLine(brush, x + w * 0.5f, y, x + w * 0.5f, y + h, w, color);
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

        // Scale srcW x srcH down to drawW x drawH, rotate about the scaled
        // quad's centre, translate so that centre lands on (centreX, centreY).
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
