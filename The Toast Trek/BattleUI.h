#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include "Line.h"
#include "Font.h"
#include "BattleButton.h"
#include "GameState.h"
#include "Inventory.h"

class BattleUI {
private:
	IDirect3DDevice9* d3dDevice;
	// Battle border
	float posX;
	float posY;
	float width;
	float height;

	Line* topLine;
	Line* bottomLine;
	Line* leftLine;
	Line* rightLine;

	BattleButton* fightButton;
	BattleButton* actButton;
	BattleButton* itemButton;
	BattleButton* mercyButton;

	// When button select
	int selectedButton;
	// Act menu button option: 1. act cute, 2. roll on ground, 3. bark
	int selectedActOption;
	bool actChoiceUsed[3];

	//Button when hovered
	bool fightHovered;
	bool actHovered;
	bool itemHovered;
	bool mercyHovered;
	bool mouseWasDown;
	bool menuMouseWasDown;

	bool fled;
	int fightDamage;
	int itemHealAmount;

	// "You have encountered <enemyName>!" - set per fight by BattleState
	std::string enemyName = "the enemy";
	bool showEncounterMessage;
	std::string encounterMessage;
	bool showActChoices;
	bool showItemChoices;
	int selectedItemOption;
	int healthPotionCount;
	int boneCount;
	int toastCount;

	Font* encounterFont;
	Font* actMenuFont;


	void DrawButtonBackground(const RECT& rect, D3DCOLOR color);
	void DrawButtonBorder(const RECT& rect, D3DCOLOR color);
	bool IsPointOverButton(float pointX, float pointY, Sprite* button) const;
	

public: 
	BattleUI(IDirect3DDevice9* d3dDevice);
	~BattleUI();
	void SelectButton(int direction);
	int GetSelectButton(GameContext& context);
	int GetActSelection(GameContext& context);
	void SetEnemyName(const std::string& name);

	void SetShowEncounterMessage(bool show);
	void SetShowActChoices(bool show);
	void SetActChoiceUsed(int index, bool used);
	void SetShowItemChoices(bool show, const Inventory* inventory = nullptr);
	int GetItemSelection(GameContext& context);
	void Render(LPD3DXSPRITE sharedBrush);

	void UpdateMenuButtons(GameContext& context);
	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;

};
