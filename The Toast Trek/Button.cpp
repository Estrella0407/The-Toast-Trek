#include "Button.h"

Button::Button(IDirect3DDevice9* device, const std::string& label,
               int x, int y, int width, int height, int fontSize)
    : label(label),
      colFill(D3DCOLOR_XRGB(255, 255, 255)),
      colFillHover(D3DCOLOR_XRGB(255, 225, 120)),
      colFillDisabled(D3DCOLOR_XRGB(90, 90, 90)),
      colBorder(D3DCOLOR_XRGB(0, 0, 0)),
      colText(D3DCOLOR_XRGB(0, 0, 0)),
      colTextDisabled(D3DCOLOR_XRGB(150, 150, 150)),
      hovered(false), enabled(true), prevMouseDown(false)
{
    rect.left = x;
    rect.top = y;
    rect.right = x + width;
    rect.bottom = y + height;

    // Font's rect = the button's rect, so Draw(text, colour) centres the label
    font = new Font(device, (float)x, (float)y, width, height, fontSize, "Arial");

    const float x0 = (float)x, y0 = (float)y;
    const float x1 = (float)(x + width), y1 = (float)(y + height);
    borderTop    = new Line(device, x0, y0, x1, y0);
    borderBottom = new Line(device, x0, y1, x1, y1);
    borderLeft   = new Line(device, x0, y0, x0, y1);
    borderRight  = new Line(device, x1, y0, x1, y1);
}

Button::~Button()
{
    delete font;
    delete borderTop;
    delete borderBottom;
    delete borderLeft;
    delete borderRight;
}

void Button::SetColours(D3DCOLOR fill, D3DCOLOR fillHover, D3DCOLOR border, D3DCOLOR text)
{
    colFill = fill;
    colFillHover = fillHover;
    colBorder = border;
    colText = text;
}

bool Button::Contains(float mouseX, float mouseY) const
{
    return mouseX >= (float)rect.left && mouseX <= (float)rect.right &&
           mouseY >= (float)rect.top  && mouseY <= (float)rect.bottom;
}

bool Button::Update(float mouseX, float mouseY, bool mouseDown)
{
    hovered = enabled && Contains(mouseX, mouseY);

    // A click is the frame the button goes from up to down while hovered
    const bool clicked = hovered && mouseDown && !prevMouseDown;
    prevMouseDown = mouseDown;
    return clicked;
}

void Button::Draw(IDirect3DDevice9* device, LPD3DXSPRITE brush)
{
    if (device == NULL) return;

    // Fill: clear just this rectangle of the back buffer
    D3DRECT area = { rect.left, rect.top, rect.right, rect.bottom };
    D3DCOLOR fill = !enabled ? colFillDisabled : (hovered ? colFillHover : colFill);
    device->Clear(1, &area, D3DCLEAR_TARGET, fill, 1.0f, 0);

    // Border
    borderTop->Draw(colBorder);
    borderBottom->Draw(colBorder);
    borderLeft->Draw(colBorder);
    borderRight->Draw(colBorder);

    // Label, centred in the button's rect
    font->Draw(label.c_str(), enabled ? colText : colTextDisabled, brush);
}
