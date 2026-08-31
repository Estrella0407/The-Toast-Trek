#include "Font.h"

Font::Font(IDirect3DDevice9* d3dDevice, float startX, float startY, int width, int height, int fontSize, const char* fontFace) {
    font = NULL;

    D3DXCreateFont(
        d3dDevice,                      // Device
        fontSize,                       // Height
        0,                              // Width
        0,                              // Weight
        1,                              // MipLevels
        false,                          // Italic
        DEFAULT_CHARSET,                // CharSet
        OUT_TT_ONLY_PRECIS,             // OutputPrecision
        DEFAULT_QUALITY,                // Quality
        DEFAULT_PITCH | FF_DONTCARE,    // PitchAndFamily
        fontFace,                       // pFaceName
        &font                           // Font pointer destination
    );

    // Define the initial dimensions of the text bounding box
    rect.left = (long)startX;
    rect.top = (long)startY;
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;
}

Font::~Font() {
    if (font != NULL) {
        font->Release();
        font = NULL;
    }
}

void Font::Draw(const char* text, D3DCOLOR color, LPD3DXSPRITE sprite) {
    if (font != NULL && text != NULL) {
        font->DrawTextA(
            sprite,
            text,
            -1, // Count
            &rect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP,
            color
        );
    }
}

void Font::Draw(const char* text, float x, float y, D3DCOLOR color, LPD3DXSPRITE sprite) {
    if (font != NULL && text != NULL) {
        rect.left = (long)x;
        rect.top = (long)y;

        font->DrawTextA(
            sprite,
            text,
            -1, // Count
            &rect,
            DT_NOCLIP,
            color
        );
    }
}

RECT Font::GetRect() const {
    return rect;
}