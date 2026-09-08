#include "Font.h"

Font::Font(IDirect3DDevice9* d3dDevice, float startX, float startY, int width, int height, int fontSize, const char* fontFace, int weight) {
    font = NULL;

    D3DXCreateFont(
        d3dDevice,                      // Device
        fontSize,                       // height
        0,                              // width
        weight,                         // weight (FW_BOLD by default)
        1,                              // MipLevels
        false,                          // italic
        DEFAULT_CHARSET,                // CharSet
        OUT_TT_ONLY_PRECIS,             // OutputPrecision
        ANTIALIASED_QUALITY,            // quality - smoother edges on the heavier strokes
        DEFAULT_PITCH | FF_DONTCARE,    // PitchAndFamily
        fontFace,                       // PFaceName
        &font                           // font pointer destination
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
		// Keep the original box size when moving dynamic text\
        // Updating only left/top can leave right < left at world position
        // producing an invalid RECT and causing Direct3D to draw no text
		const LONG width = rect.right - rect.left;
		const LONG height = rect.bottom - rect.top;
        rect.left = (long)x;
        rect.top = (long)y;
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;

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
