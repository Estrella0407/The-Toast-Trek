#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Line.h"
#include "Font.h"
#include "BattleButton.h"
#include "GameState.h" //GameContext (mouse input)


class BattleUI {
private:
	IDirect3DDevice9* d3dDevice;
	//battle border
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

	//when button select
	int selectedButton;

	//button when hovered
	bool fightHovered;
	bool actHovered;
	bool itemHovered;
	bool mercyHovered;
	bool mouseWasDown;

	bool fled;
	int fightDamage;
	int itemHealAmount;

	//You have encounter Skullie!
	bool showEncounterMessage;

	Font* fightFont;
	Font* actFont;
	Font* itemFont;
	Font* mercyFont;
	Font* encounterFont;


	void DrawButtonBackground(const RECT& rect, D3DCOLOR color);
	void DrawButtonBorder(const RECT& rect, D3DCOLOR color);
	bool IsPointOverButton(float pointX, float pointY, Sprite* button) const;
	

public: 
	BattleUI(IDirect3DDevice9* d3dDevice);
	~BattleUI();
	void SelectButton(int direction);
	int GetSelectButton(GameContext& context);
	void SetShowEncounterMessage(bool show);
	void Render(LPD3DXSPRITE sharedBrush);

	void UpdateMenuButtons(GameContext& context);
	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;

};
