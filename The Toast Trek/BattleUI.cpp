#include "BattleUI.h"
#include "Line.h"
#include "Font.h"
#include "BattleButton.h"
#include "GameState.h"
#include <string>
#include <iostream>

BattleUI::BattleUI(IDirect3DDevice9* d3dDevice) : d3dDevice(d3dDevice), selectedButton(-1),
	mouseWasDown(false), menuMouseWasDown(false) {

	selectedButton = 0;
	showEncounterMessage = true;
	showActChoices = false;
	showItemChoices = false;
	selectedItemOption = -1;
	healthPotionCount = 0;
	boneCount = 0;
	toastCount = 0;
	selectedActOption = -1;

	actChoiceUsed[0] = false;
	actChoiceUsed[1] = false;
	actChoiceUsed[2] = false;

	fightHovered = false;
	actHovered = false;
	itemHovered = false;
	mercyHovered = false;
	mouseWasDown = false;

	//Drawing battle box position
	posX = 300.0f;
	posY = 200.0f;
	width = 700.0f;
	height = 300.0f;

	//Act menu option button vertical position
	const float actX = 350.0f;
	const float actY = 300.0f;
	const float optionHeight = 40.0f;

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

	//D3DXCreateFont(d3dDevice, 24, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY,
	//	DEFAULT_PITCH | FF_DONTCARE, "Arial", &actMenuFont);
	actMenuFont = new Font(d3dDevice, 350.0f, 280.0f, 500, 40, 25, "Arial");

}

void BattleUI::SetShowEncounterMessage(bool show) {
	showEncounterMessage = show;
}

void BattleUI::SetEnemyName(const std::string& name) {
	if (!name.empty()) enemyName = name;
}
void BattleUI::SetShowActChoices(bool show) {
	showActChoices = show;
	if (!show) selectedActOption = -1;
}

void BattleUI::SetShowItemChoices(bool show, const Inventory* inventory) {
	showItemChoices = show;
	selectedItemOption = -1;
	if (inventory != nullptr) {
		healthPotionCount = inventory->GetCount(ItemType::HealthPotion);
		boneCount = inventory->GetCount(ItemType::Bone);
		toastCount = inventory->GetCount(ItemType::Toast);
	}
}

int BattleUI::GetItemSelection(GameContext& context) {
	const float x = 350.0f;
	const float y[3] = { 280.0f, 325.0f, 370.0f };
	selectedItemOption = -1;
	for (int i = 0; i < 3; ++i) {
		if (context.mouseX >= x && context.mouseX <= x + 500.0f &&
			context.mouseY >= y[i] && context.mouseY <= y[i] + 40.0f) {
			selectedItemOption = i;
		}
	}
	const bool clicked = context.mouseLeftDown && !menuMouseWasDown;
	menuMouseWasDown = context.mouseLeftDown;
	return clicked ? selectedItemOption : -1;
}

void BattleUI::SetActChoiceUsed(int index, bool used) {
	if (index >= 0 && index < 3) 
		actChoiceUsed[index] = used;
}

int BattleUI::GetActSelection(GameContext& context) {
	const float x = 350.0f;
	const float y[3] = { 280.0f, 325.0f, 370.0f };
	const float optionWidth = 500.0f;
	const float optionHeight = 40.0f;

	selectedActOption = -1;
	for (int i = 0; i < 3; ++i) {
		if (context.mouseX >= x && context.mouseX <= x + optionWidth &&
			context.mouseY >= y[i] && context.mouseY <= y[i] + optionHeight) {
			selectedActOption = i;
			break;
		}
	}

	const bool clicked = context.mouseLeftDown && !menuMouseWasDown;
	menuMouseWasDown = context.mouseLeftDown;
	return clicked ? selectedActOption : -1;

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

//Bool BattleUI::IsPointOverButton(float pointX, float pointY, Sprite* button) const {
//	if (button == nullptr) return false;
//
//	// Button art is a 256x256 canvas with a much smaller pill-shaped label
//	// centered in it; hit-testing the full canvas would make adjacent
//	// buttons' clickable areas overlap (they're only spaced 200px apart)
//	const float hitWidth = 180.0f;
//	const float hitHeight = 100.0f;
//
//	D3DXVECTOR2 topLeft = button->GetPosition();
//	float centerX = topLeft.x + 128.0f;
//	float centerY = topLeft.y + 128.0f;
//
//	Return pointX >= centerX - hitWidth * 0.5f && pointX <= centerX + hitWidth * 0.5f &&
//		PointY >= centerY - hitHeight * 0.5f && pointY <= centerY + hitHeight * 0.5f;
//}

int BattleUI::GetSelectButton(GameContext& context) {
	bool clicked = context.mouseLeftDown && !mouseWasDown;
	mouseWasDown = context.mouseLeftDown;
	if (!clicked) return -1;
	if (fightButton->IsHovered()) return 0;
	if (actButton->IsHovered()) return 1;
	if (itemButton->IsHovered()) return 2;
	if (mercyButton->IsHovered()) return 3;
	return -1;
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
}

BattleUI::~BattleUI() {
	delete topLine;
	delete bottomLine;
	delete leftLine;
	delete rightLine;

	delete fightButton;
	delete actButton;
	delete itemButton;
	delete mercyButton;
	delete encounterFont;
	delete actMenuFont;
}

void BattleUI::Render(LPD3DXSPRITE sharedBrush) {
	D3DCOLOR black = D3DCOLOR_XRGB(0, 0, 0);
	D3DCOLOR white = D3DCOLOR_XRGB(255, 255, 255);
	D3DCOLOR yellow = D3DCOLOR_XRGB(255, 225, 120);

	topLine->Draw(black);
	bottomLine->Draw(black);
	leftLine->Draw(black);
	rightLine->Draw(black);

	if (showActChoices) {
		if (!actChoiceUsed[0]) {
			actMenuFont->Draw("1. Act Cute", 350.0f, 280.0f, selectedActOption == 0 ? yellow : black);
		}
		if (!actChoiceUsed[1]) {
			actMenuFont->Draw("2. Roll on Ground", 350.0f, 325.0f, selectedActOption == 1 ? yellow : black);
		}
		if (!actChoiceUsed[2]) {
			actMenuFont->Draw("3. Bark", 350.0f, 370.0f, selectedActOption == 2 ? yellow : black);
		}
	}
	if (showItemChoices) {
		std::string potion = "1. Health Potion x" + std::to_string(healthPotionCount);
		std::string bone = "2. Bone x" + std::to_string(boneCount);
		std::string toastLabel = "3. Toast x" + std::to_string(toastCount);
		actMenuFont->Draw(potion.c_str(), 350.0f, 280.0f,
			selectedItemOption == 0 ? yellow : black);
		actMenuFont->Draw(bone.c_str(), 350.0f, 325.0f,
			selectedItemOption == 1 ? yellow : black);
		actMenuFont->Draw(toastLabel.c_str(), 350.0f, 370.0f,
			selectedItemOption == 2 ? yellow : black);
	}

	fightButton->Render();
	actButton->Render();
	itemButton->Render();
	mercyButton->Render();

	if (showEncounterMessage) {
		std::string msg = "You have encounter " + enemyName + "!";
		encounterFont->Draw(msg.c_str(), black);
	}
}
