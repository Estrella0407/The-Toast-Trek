#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "Font.h"
#include "Line.h"

// A reusable rectangular button. Any scene creates one, positions it, gives
// it a label (and optionally colours / enabled state), then each frame sets
// its hover or selected state and calls Render(). Built from the taught
// primitives: device->Clear for the fill, Line for the border, Font for the
// centred label - the same recipe the battle menu buttons use.
class Button {
public:
    Button(IDirect3DDevice9* device, const char* label,
           int x, int y, int width, int height, int fontSize = 25);
    ~Button();

    Button(const Button&) = delete;             // owns Font* / Line*
    Button& operator=(const Button&) = delete;

    // --- setup ---
    void SetLabel(const char* text) { label = text; }
    void SetEnabled(bool on) { enabled = on; }
    void SetColours(D3DCOLOR fill, D3DCOLOR fillActive, D3DCOLOR border, D3DCOLOR text);

    // --- per-frame state ---
    // Pure hit-test, no state change.
    bool IsHovered(float mouseX, float mouseY) const;
    void SetHovered(bool value) { hovered = value; }   // mouse
    void SetSelected(bool value) { selected = value; } // keyboard cursor

    // Convenience: refresh hover from the cursor and report a fresh click
    // (mouse pressed this frame over an enabled button).
    bool Update(float mouseX, float mouseY, bool mouseDown);

    void Render(LPD3DXSPRITE brush = NULL);

    // --- queries ---
    bool IsHovered() const { return hovered; }
    bool IsSelected() const { return selected; }
    bool IsEnabled() const { return enabled; }
    RECT GetRect() const { return rect; }

private:
    IDirect3DDevice9* device;
    RECT rect;
    std::string label;

    Font* font;                     // owned - draws the label centred in `rect`
    Line* borderTop;                // owned
    Line* borderBottom;
    Line* borderLeft;
    Line* borderRight;

    D3DCOLOR colFill;
    D3DCOLOR colFillActive;         // hovered or selected
    D3DCOLOR colFillDisabled;
    D3DCOLOR colBorder;
    D3DCOLOR colText;
    D3DCOLOR colTextDisabled;

    bool hovered;
    bool selected;
    bool enabled;
    bool prevMouseDown;
};
