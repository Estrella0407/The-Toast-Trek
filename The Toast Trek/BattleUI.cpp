#include "BattleUI.h"
#include "Line.h"
#include "Font.h"
#include "BattleButton.h"
#include "GameState.h"
#include <string>
#include <iostream>

BattleUI::BattleUI(IDirect3DDevice9* d3dDevice) : selectedButton(-1), mouseWasDown(false) {

	selectedButton = 0;
	showEncounterMessage = true;
	fightHovered = false;
	actHovered = false;
	itemHovered = false;
	mercyHovered = false;
	mouseWasDown = false;

	posX = 300.0f;
	posY = 200.0f;
	width = 700.0f;
	height = 300.0f;

	topLine = new Line(d3dDevice, posX, posY, posX + width, posY);
	bottomLine = new Line(d3dDevice, posX, posY + height, posX + width, posY + height);
	leftLine = new Line(d3dDevice, posX, posY, posX, posY + height);
	rightLine = new Line(d3dDevice, posX + width, posY, posX + width, posY + height);

	fightButton = new BattleButton(d3dDevice, "FIGHT", 275, 600, 150, 50);
	actButton = new BattleButton(d3dDevice, "ACT", 475, 600, 150, 50);
	itemButton = new BattleButton(d3dDevice, "ITEM", 675, 600, 150, 50);
	mercyButton = new BattleButton(d3dDevice, "MERCY", 875, 600, 150, 50);

	encounterFont = new Font(d3dDevice, 350, 350, 600, 100, 30, "Arial");
	mouseWasDown = false;
	fled = false;
	fightDamage = 10;
	itemHealAmount = 5;

}

void BattleUI::SetShowEncounterMessage(bool show) {
	showEncounterMessage = show;
}

void BattleUI::DrawButtonBorder(const RECT& rect, D3DCOLOR color) {
	Line top(d3dDevice, (float)rect.left, (float)rect.top, (float)rect.right, (float)rect.top);
	Line bottom(d3dDevice, (float)rect.left, (float)rect.bottom, (float)rect.right, (float)rect.bottom);
	Line left(d3dDevice, (float)rect.left, (float)rect.top, (float)rect.left, (float)rect.bottom);
	Line right(d3dDevice, (float)rect.right, (float)rect.top, (float)rect.right, (float)rect.bottom);
	
	top.Draw(color);
	bottom.Draw(color);
	left.Draw(color);
	right.Draw(color);
}

//bool BattleUI::IsPointOverButton(float pointX, float pointY, Sprite* button) const {
//	if (button == nullptr) return false;
//
//	// Button art is a 256x256 canvas with a much smaller pill-shaped label
//	// centered in it; hit-testing the full canvas would make adjacent
//	// buttons' clickable areas overlap (they're only spaced 200px apart).
//	const float hitWidth = 180.0f;
//	const float hitHeight = 100.0f;
//
//	D3DXVECTOR2 topLeft = button->GetPosition();
//	float centerX = topLeft.x + 128.0f;
//	float centerY = topLeft.y + 128.0f;
//
//	return pointX >= centerX - hitWidth * 0.5f && pointX <= centerX + hitWidth * 0.5f &&
//		pointY >= centerY - hitHeight * 0.5f && pointY <= centerY + hitHeight * 0.5f;
//}

int BattleUI::GetSelectButton(GameContext& context) {
	std::cout << "GetSelectButton called!" << std::endl;
	std::cout << "Mouse:" 
		<< context.mouseX << "," 
		<< context.mouseY << std::endl;

	std::cout << "Mouse left down: " 
		<< context.mouseLeftDown << std::endl;

	bool clicked = context.mouseLeftDown && !mouseWasDown;
	mouseWasDown = context.mouseLeftDown;

	if (!clicked)
		return -1;
	OutputDebugStringA("MOUSE CLICK DETECTED!\n");

	if (!context.mouseLeftDown) {
		std::cout << "Mouse is NOT down" << std::endl;
		return -1;

		if (fightButton->IsHovered())
			OutputDebugStringA("FIGHT CLICK DETECTED!\n");
			return 0;
		if (actButton->IsHovered())
			OutputDebugStringA("ACT CLICK DETECTED!\n");
			return 1;
		if (itemButton->IsHovered())
			OutputDebugStringA("ITEM CLICK DETECTED!\n");
			return 2;
		if (mercyButton->IsHovered())
			fled = true;
			OutputDebugStringA("MERCY CLICK DETECTED!\n");
			return 3;
		return -1;

	}
}

void BattleUI::UpdateMenuButtons(GameContext& context) {

	bool fightOver = fightButton->IsHovered(context.mouseX, context.mouseY);
	bool actOver = actButton->IsHovered(context.mouseX, context.mouseY);
	bool itemOver = itemButton->IsHovered(context.mouseX, context.mouseY);
	bool mercyOver = mercyButton->IsHovered(context.mouseX, context.mouseY);

	fightButton->SetHovered(fightOver);
	actButton->SetHovered(actOver);
	itemButton->SetHovered(itemOver);
	mercyButton->SetHovered(mercyOver);

	bool clicked = context.mouseLeftDown && !mouseWasDown;
	mouseWasDown = context.mouseLeftDown;

	selectedButton: -1;
	if (clicked) {
		if (fightOver)
			selectedButton = 0;
		else if (actOver)
			selectedButton = 1;
		else if (itemOver)
			selectedButton = 2;
		else if (mercyButton)
			selectedButton = 3;
	}

	OutputDebugStringA("Battle UI::UpdateMenuButtons called\n");

	//if (fightHovered) {
	//	//enemy->TakeDamage(fightDamage);
	//	OutputDebugStringA(("Enemy hit for " + std::to_string(fightDamage) +
	//		", health now " + std::to_string(enemy->GetHealth()) + "\n").c_str());
	//}
	//else if (itemHovered) {
	//	heart->Heal(itemHealAmount);
	//}
	//else if (mercyHovered) {
	//	fled = true;
	//}
	//else if (actHovered) {
	//	// ACT has no per-boss dialogue/options defined yet - clickable, but a no-op for now.
	//}

	if (fightOver) {
		OutputDebugStringA(("FIGHT HOVER - Mouse: " + std::to_string(context.mouseX) + "," + std::to_string(context.mouseY) + "\n").c_str());

		RECT r = fightButton->GetRect();
		OutputDebugStringA(("FIGHT RECT : " + std::to_string(r.left) + ", " +
			std::to_string(r.top) + "," + std::to_string(r.right) + ", " + std::to_string(r.bottom) + "\n").c_str());
	}
}

BattleUI::~BattleUI() {
	delete topLine;
	delete bottomLine;
	delete leftLine;
	delete rightLine;

	delete fightFont;
	delete actFont;
	delete itemFont;
	delete mercyFont;
}

void BattleUI::Render(LPD3DXSPRITE sharedBrush) {
	D3DCOLOR black = D3DCOLOR_XRGB(0, 0, 0);
	//D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	//D3DCOLOR green = D3DCOLOR_XRGB(85, 107, 47);

	topLine->Draw(black);
	bottomLine->Draw(black);
	leftLine->Draw(black);
	rightLine->Draw(black);

	fightButton->Render();
	actButton->Render();
	itemButton->Render();
	mercyButton->Render();

}
