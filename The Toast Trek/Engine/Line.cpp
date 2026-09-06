#include "Line.h"

Line::Line(IDirect3DDevice9* d3dDevice, float startX, float startY, float endX, float endY) {
    line = NULL;
    startPos = D3DXVECTOR2(startX, startY);
    endPos = D3DXVECTOR2(endX, endY);

    D3DXCreateLine(d3dDevice, &line);
}

Line::~Line() {
    if (line != NULL) {
        line->Release();
        line = NULL;
    }
}

void Line::Draw(D3DCOLOR colorTint) {
    if (line != NULL) {
        D3DXVECTOR2 vertices[] = { startPos, endPos };

        line->Begin();
        line->Draw(vertices, 2, colorTint);
        line->End();
    }
}
