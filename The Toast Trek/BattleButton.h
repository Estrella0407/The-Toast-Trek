#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Line.h"
#include "Font.h"

class BattleButton {
private:
	IDirect3DDevice9* d3dDevice;
	RECT rect;
	Font* font;

	Line* topLine;
	Line* bottomLine;
	Line* leftLine;
	Line* rightLine;

	const char* text;
	bool hovered;
	bool selected;

	void DrawBackground(D3DCOLOR color);

public:
	BattleButton(IDirect3DDevice9* d3dDevice, const char* buttonText, int x, int y, int width, int height);
	~BattleButton();
	bool IsHovered(float mouseX, float mouseY) const;
	void SetHovered(bool value);
	void SetSelected(bool value);
	bool IsHovered() const;
	bool IsSelected() const;

	RECT GetRect() const;
	void Render();
};
