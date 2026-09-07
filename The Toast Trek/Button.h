#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "Font.h"
#include "Line.h"

// A reusable rectangular button. Any scene can create one, position it, give
// it a label and colours, then each frame call Update() with the mouse state
// and Draw() to paint it. Built from the taught primitives: device->Clear
// for the fill, Line for the border, Font for the centred label.
class Button {
public:
    Button(IDirect3DDevice9* device, const std::string& label,
           int x, int y, int width, int height, int fontSize = 22);
    ~Button();

    Button(const Button&) = delete;             // owns Font* / Line*
    Button& operator=(const Button&) = delete;

    // --- setup ---
    void SetLabel(const std::string& text) { label = text; }
    void SetEnabled(bool on) { enabled = on; }
    void SetColours(D3DCOLOR fill, D3DCOLOR fillHover, D3DCOLOR border, D3DCOLOR text);

    // --- per frame ---
    // Refresh the hover state from the cursor and report a fresh click
    // (mouse pressed this frame while over an enabled button).
    bool Update(float mouseX, float mouseY, bool mouseDown);

    // For keyboard-driven menus: light the button up without a cursor.
    void SetHovered(bool on) { hovered = on; }

    void Draw(IDirect3DDevice9* device, LPD3DXSPRITE brush);

    // --- queries ---
    bool Contains(float mouseX, float mouseY) const;
    bool IsHovered() const { return hovered; }
    bool IsEnabled() const { return enabled; }
    RECT GetRect() const { return rect; }

private:
    RECT rect;
    std::string label;

    Font* font;                     // owned - draws the label centred in `rect`
    Line* borderTop;                // owned
    Line* borderBottom;
    Line* borderLeft;
    Line* borderRight;

    D3DCOLOR colFill;
    D3DCOLOR colFillHover;
    D3DCOLOR colFillDisabled;
    D3DCOLOR colBorder;
    D3DCOLOR colText;
    D3DCOLOR colTextDisabled;

    bool hovered;
    bool enabled;
    bool prevMouseDown;
};
