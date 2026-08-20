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

    void DrawClockFace(float centerX, float centerY, float outerRadius, float innerRadius);
    void DrawTimeLines(float centerX, float centerY, float length, float angleDegrees, D3DCOLOR color = D3DCOLOR_XRGB(255, 255, 255));
    void Draw(D3DCOLOR colorTint = D3DCOLOR_XRGB(255, 255, 255));
};