#include "Line.h"
#include <cmath>

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

void Line::DrawClockFace(float centerX, float centerY, float outerRadius, float innerRadius) {
    if (line == NULL) return;

    // Circle = 360 degrees. 360 / 12 hours = 30 degrees
    for (int i = 0; i < 12; i++) {
        float angleDegrees = i * 30.0f;
        float angleRadians = D3DXToRadian(angleDegrees);

        // Starting point (inner rim of the clock)
        D3DXVECTOR2 start(
            centerX + innerRadius * sin(angleRadians),
            centerY - innerRadius * cos(angleRadians)
        );

        // Ending point (outer rim of the clock)
        D3DXVECTOR2 end(
            centerX + outerRadius * sin(angleRadians),
            centerY - outerRadius * cos(angleRadians)
        );

        D3DXVECTOR2 vertices[] = { start, end };
        line->Begin();
        line->Draw(vertices, 2, D3DCOLOR_XRGB(255, 255, 255));
        line->End();
    }
}

void Line::DrawTimeLines(float centerX, float centerY, float length, float angleDegrees, D3DCOLOR color) {
    if (line == NULL) return;
    float angleRadians = D3DXToRadian(angleDegrees);

    D3DXVECTOR2 start(centerX, centerY);
    D3DXVECTOR2 end(
        centerX + length * sin(angleRadians),
        centerY - length * cos(angleRadians)
    );

    D3DXVECTOR2 vertices[] = { start, end };
    line->Begin();
    line->Draw(vertices, 2, color);
    line->End();
}

void Line::Draw(D3DCOLOR colorTint) {
    if (line != NULL) {
        D3DXVECTOR2 vertices[] = { startPos, endPos };

        line->Begin();
        line->Draw(vertices, 2, colorTint);
        line->End();
    }
}
