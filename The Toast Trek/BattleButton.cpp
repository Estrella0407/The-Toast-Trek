#include "BattleButton.h"

BattleButton::BattleButton(IDirect3DDevice9* d3dDevice, const char* buttonText, int x, int y, int width, int height) {
	this->d3dDevice = d3dDevice;
	text = buttonText;

	hovered = false;
	selected = false;

	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;

	font = new Font(d3dDevice, x, y, width, height, 25, "Arial");
	topLine = new Line(d3dDevice, (float)x, (float)y, (float)(x + width), (float)y);
	bottomLine = new Line(d3dDevice, (float)x, (float)(y + height), (float)(x + width), (float)(y + height));
	leftLine = new Line(d3dDevice, (float)x, (float)y, (float)x, (float)(y + height));
	rightLine = new Line(d3dDevice, (float)(x + width), (float)y, (float)(x + width), (float)(y + height));
}

bool BattleButton::IsHovered(int mouseX, int mouseY) const {
	return mouseX >= rect.left &&
		mouseX <= rect.right &&
		mouseY >= rect.top &&
		mouseY <= rect.bottom;
}

void BattleButton::DrawBackground(D3DCOLOR color) {
	D3DRECT background;
	background.x1 = rect.left;
	background.y1 = rect.top;
	background.x2 = rect.right;
	background.y2 = rect.bottom;

	d3dDevice->Clear(1, &background, D3DCLEAR_TARGET, color, 1.0f, 0);
}

void BattleButton::SetHovered(bool value) {
	hovered = value;
}

void BattleButton::SetSelected(bool value) {
	selected = value;
}

bool BattleButton::IsHovered() const {
	return hovered;
}

bool BattleButton::IsSelected() const {
	return selected;
}

RECT BattleButton::GetRect() const {
	return rect;
}

BattleButton::~BattleButton() {
	delete font;
	delete topLine; 
	delete bottomLine;
	delete leftLine;
	delete rightLine;
}

void BattleButton::Render() {
	D3DCOLOR black = D3DCOLOR_XRGB(0, 0, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	D3DCOLOR green = D3DCOLOR_XRGB(255, 225, 120);

	D3DCOLOR ColorBackground = hovered ? green : white;
	
	DrawBackground(ColorBackground);

	topLine->Draw(black);
	bottomLine->Draw(black);
	leftLine->Draw(black);
	rightLine->Draw(black);

	font->Draw(text, black);
}
