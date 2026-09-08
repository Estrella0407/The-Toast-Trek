#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Line {
private:
    LPD3DXLINE line;
    D3DXVECTOR2 startPos;
    D3DXVECTOR2 endPos;

public:
    Line(
        IDirect3DDevice9* d3dDevice,
        float startX, float startY,
        float endX, float endY
    );
    ~Line();

    void Draw(D3DCOLOR colorTint = D3DCOLOR_XRGB(255, 255, 255));
};