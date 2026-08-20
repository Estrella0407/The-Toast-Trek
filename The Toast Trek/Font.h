#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>

class Font {
private:
    RECT rect;
    LPD3DXFONT font;

public:
    Font(
        IDirect3DDevice9* d3dDevice,
        float startX = 0.0f,
        float startY = 0.0f,
        int width = 200,
        int height = 50,
        int fontSize = 25,
        const char* fontFace = "Arial"
    );
    ~Font();

    void Draw(const char* text, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255));
    void Draw(const char* text, float x, float y, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255));

    RECT GetRect() const;
};